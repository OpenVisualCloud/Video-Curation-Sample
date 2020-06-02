/*******************************************************************************
 * Copyright (C) 2018-2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 ******************************************************************************/
 /*******************************************************************************
 * adapted from https://github.com/opencv/gst-video-analytics/tree/v0.6/samples/cpp/object_detection_demo_yolov3_async/main.cpp
 * and
 * https://github.com/opencv/gst-video-analytics/blob/v0.6/samples/cpp/face_attributes/main.cpp
 ******************************************************************************/

// Project Includes
#include "metaData_extract.hpp"


std::vector<std::string> SplitString(const std::string input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


void ExploreDir(std::string search_dir, const std::string &model_name, std::vector<std::string> &result) {
    if (auto dir_handle = opendir(search_dir.c_str())) {
        while (auto file_handle = readdir(dir_handle)) {
            if ((!file_handle->d_name) || (file_handle->d_name[0] == '.'))
                continue;
            if (file_handle->d_type == DT_DIR)
                ExploreDir(search_dir + file_handle->d_name + "/", model_name, result);
            if (file_handle->d_type == DT_REG) {
                std::string name(file_handle->d_name);
                if (name == model_name)
                    result.push_back(search_dir + "/" + name);
            }
        }
        closedir(dir_handle);
    }
}


std::vector<std::string> FindModel(const std::vector<std::string> &search_dirs, const std::string &model_name) {
    std::vector<std::string> result;
    for (const std::string &dir : search_dirs) {
        ExploreDir(dir + "/", model_name, result);
    }
    return result;
}


std::string to_upper_case(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}


std::map<std::string, std::string> FindModels(const std::vector<std::string> &search_dirs,
                                              const std::vector<std::string> &model_names,
                                              const std::string &precision) {
    std::map<std::string, std::string> result;
    for (const std::string &model_name : model_names) {
        std::vector<std::string> model_paths = FindModel(search_dirs, model_name);
        if (model_paths.empty())
            continue;
        result[model_name] = model_paths.front();
        for (const auto &model_path : model_paths)
            // TODO extract precision from xml file
            if (to_upper_case(model_path).find(to_upper_case(precision)) != std::string::npos) {
                result[model_name] = model_path;
                break;
            }
    }
    if (result.empty())
        throw std::runtime_error("Can't find file for model");
    return result;
}


double IntersectionOverUnion(const DetectionObject &box_1, const DetectionObject &box_2) {
    double width_of_overlap_area = fmin(box_1.xmax, box_2.xmax) - fmax(box_1.xmin, box_2.xmin);
    double height_of_overlap_area = fmin(box_1.ymax, box_2.ymax) - fmax(box_1.ymin, box_2.ymin);
    double area_of_overlap;
    if (width_of_overlap_area < 0 || height_of_overlap_area < 0)
        area_of_overlap = 0;
    else
        area_of_overlap = width_of_overlap_area * height_of_overlap_area;
    double box_1_area = (box_1.ymax - box_1.ymin) * (box_1.xmax - box_1.xmin);
    double box_2_area = (box_2.ymax - box_2.ymin) * (box_2.xmax - box_2.xmin);
    double area_of_union = box_1_area + box_2_area - area_of_overlap;
    return area_of_overlap / area_of_union;
}


static int EntryIndex(int side, int lcoords, int lclasses, int location, int entry){
    int n = location / (side * side);
    int loc = location % (side * side);
    return n * side * side * (lcoords + lclasses + 1) + entry * side * side + loc;
}


void ParseYOLOV3Output(GVA::Tensor &tensor_yolo, int image_width, int image_height,
                       std::vector<DetectionObject> &objects, gdouble threshold) {
    const int out_blob_h = tensor_yolo.dims()[2];
    constexpr int coords = 4;
    constexpr int num = 3;
    constexpr int classes = 80;
    const std::vector<float> anchors = {10.0, 13.0, 16.0,  30.0,  33.0, 23.0,  30.0,  61.0,  62.0,
                                        45.0, 59.0, 119.0, 116.0, 90.0, 156.0, 198.0, 373.0, 326.0};
    int side = out_blob_h;
    int anchor_offset = 0;
    switch (side) {
    case 13:
        anchor_offset = 2 * 6;
        break;
    case 26:
        anchor_offset = 2 * 3;
        break;
    case 52:
        anchor_offset = 2 * 0;
        break;
    default:
        throw std::runtime_error("Invalid output size");
    }
    int side_square = side * side;
    
    // const float *output_blob = (const float *)RegionYolo->data;
    std::vector<float> output_blob = tensor_yolo.data<float>();
        for (int i = 0; i < side_square; ++i) {
        int row = i / side;
        int col = i % side;
        for (int n = 0; n < num; ++n) {
            int obj_index = EntryIndex(side, coords, classes, n * side * side + i, coords);
            int box_index = EntryIndex(side, coords, classes, n * side * side + i, 0);

            float scale = output_blob[obj_index];
            if (scale < threshold)
                continue;
            double x = (col + output_blob[box_index + 0 * side_square]) / side * 416;
            double y = (row + output_blob[box_index + 1 * side_square]) / side * 416;

            double width = std::exp(output_blob[box_index + 2 * side_square]) * anchors[anchor_offset + 2 * n];
            double height = std::exp(output_blob[box_index + 3 * side_square]) * anchors[anchor_offset + 2 * n + 1];

            for (int j = 0; j < classes; ++j) {
                int class_index = EntryIndex(side, coords, classes, n * side_square + i, coords + 1 + j);
                float prob = scale * output_blob[class_index];
                if (prob < threshold)
                    continue;
                DetectionObject obj(x, y, height, width, j, prob,
                                    static_cast<float>(image_height) / static_cast<float>(416),
                                    static_cast<float>(image_width) / static_cast<float>(416));
                objects.push_back(obj);
            }
        }
    }
}


void DrawObjects(std::vector<DetectionObject> &objects, cv::Mat &frame, VDMSConverter *converter, std::ofstream *outFD,
                 std::string video_id, int frameID, bool streamVid, bool noDisplay, std::vector<std::string> objIDArr){
    std::sort(objects.begin(), objects.end());
    for (size_t i = 0; i < objects.size(); ++i){
        if (objects[i].confidence == 0) 
            continue;
        for (size_t j = i + 1; j < objects.size(); ++j) 
            if (IntersectionOverUnion(objects[i], objects[j]) >= 0.4)
                objects[j].confidence = 0;
    }        
    // Drawing boxes
    for (const auto &object : objects) {
        if (object.confidence < 0) 
            continue;
        guint label = object.class_id;
        float confidence = object.confidence;
        
        if (confidence > 0 && std::find(objIDArr.begin(), objIDArr.end(), coco_labels[label]) != objIDArr.end()) 
        {
            // Drawing only objects when >confidence_threshold probability
            // Do not draw when noDisplay is TRUE
            if (!noDisplay) {
                std::ostringstream conf;
                conf << ":" << std::fixed << std::setprecision(3) << confidence;
                cv::putText(frame,
                        (label < coco_labels.size() ? coco_labels[label] : std::string("label #") + std::to_string(label)) + conf.str(),
                        cv::Point2f(object.xmin, object.ymin - 5), cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(0, 0, 255));
                cv::rectangle(frame, cv::Point2f(object.xmin, object.ymin), cv::Point2f(object.xmax, object.ymax),
                          cv::Scalar(0, 255, 0), 7);
	    }

            std::string metadata_str = std::to_string(frameID) + ","
                                        + std::to_string(object.xmin) + ","
                                        + std::to_string(object.ymin) + "," 
                                        + std::to_string(object.xmax - object.xmin) + ","
                                        + std::to_string(object.ymax - object.ymin) + ","
                                        + coco_labels[label];
            if(converter != NULL) 
            {
                bbox_obj bbox;
                bbox.props[VDMS_OBJID] = coco_labels[label];
                bbox.props[VDMS_FRAMEID] = frameID;
                bbox.x = object.xmin;
                bbox.y = object.ymin;
                bbox.w = object.xmax - object.xmin;
                bbox.h = object.ymax - object.ymin;
                bbox.props[VDMS_FRAMEID] = frameID;
                if(converter->upload_data(bbox, frameID, video_id, streamVid) != 0) 
                {
                    std::cout << "Upload Failed" << std::endl;
                }
            }
            if(outFD != NULL) 
            {
                outFD[0] << metadata_str << std::endl;
                outFD->flush();
            }
            std::cout << metadata_str << std::endl;
        }
    }
    std::cout << video_id << " " << "frame " << frameID << " processed" << std::endl;
}


static GstPadProbeReturn pad_probe_callback_face(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    pad_probe_CB_args *args = (pad_probe_CB_args*)user_data;
    VDMSConverter *converter = args->converter;
    std::ofstream *outFD = args->outFD;
    int *frameID = args->frameID;
    
    auto buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    
    // Making a buffer writable can fail (for example if it cannot be copied and is used more than once)
    // buffer = gst_buffer_make_writable(buffer);
    if (buffer == NULL)
        return GST_PAD_PROBE_OK;
    
    // Get width and height
    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (!caps)
        throw std::runtime_error("Can't get current caps");

    GVA::VideoFrame video_frame(buffer, caps);
    gint width = video_frame.video_info()->width;
    gint height = video_frame.video_info()->height;
    
    // Map buffer and create OpenCV image
    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) 
        return GST_PAD_PROBE_OK;
    cv::Mat mat(height, width, CV_8UC4, map.data);
    
    // Iterate detected objects and all attributes
    for (GVA::RegionOfInterest &roi : video_frame.regions()) {
        auto meta = roi.rect(); //roi.meta();
        std::string label;
        std::string metadata_str;
        std::string gender;
        std::string age;
        std::string emotion;
        for (auto tensor : roi.tensors()) {
            std::string model_name = tensor.model_name();
            std::string layer_name = tensor.layer_name();
            std::vector<float> data = tensor.data<float>();            
            if (model_name.find("gender") != std::string::npos && layer_name.find("prob") != std::string::npos) {
                label += (data[1] > 0.5) ? " M " : " F ";
                gender = (data[1] > 0.5) ? "male" : "female";
            }
            
            if (layer_name.find("age") != std::string::npos) {
                label += std::to_string((int)(data[0] * 100));
                age = std::to_string((int)(data[0] * 100));
            }
            
            if (model_name.find("EmoNet") != std::string::npos) {
                static const std::vector<std::string> emotionsDesc = {"neutral", "happy", "sad", "surprise", "anger"};
                int index = max_element(begin(data), end(data)) - begin(data);
                label += " " + emotionsDesc[index];
                emotion = emotionsDesc[index];
            }
        }
        if (!label.empty()) {
            std::string metadata_str = std::to_string(frameID[0]) + ","
                                        + std::to_string(meta.x) + ","
                                        + std::to_string(meta.y) + "," 
                                        + std::to_string(meta.w) + ","
                                        + std::to_string(meta.h) + ","
                                        + age + ","
                                        + emotion + ","
                                        + gender;
            if(converter != NULL ) {
               bbox_obj bbox;
                VDMSConverter::add_bbox_props(std::stoi(age), bbox, emotion, frameID[0], gender);
                bbox.x = meta.x;
                bbox.y = meta.y;
                bbox.w = meta.w;
                bbox.h = meta.h;
                if(converter->upload_data(bbox, frameID[0], args->video_id, args->streamVid) != 0) 
                    std::cout << "Upload Failed" << std::endl;
            }
            if(outFD != NULL) {
                outFD[0] << metadata_str << std::endl;
                outFD->flush();
            }
            std::cout << metadata_str << std::endl;
        }
    }
    
    // Release the memory previously mapped with gst_buffer_map
    gst_buffer_unmap(buffer, &map);
    // Unref a GstCaps and and free all its structures and the structures' values
    gst_caps_unref(caps);
    GST_PAD_PROBE_INFO_DATA(info) = buffer;
    std::cout << args->video_id << " " << "frame " << frameID[0] << " processed" << std::endl;
    frameID[0]++;
    return GST_PAD_PROBE_OK;
}


static GstPadProbeReturn pad_probe_callback_obj(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    pad_probe_CB_args *args = (pad_probe_CB_args*)user_data;
    VDMSConverter *converter = args->converter;
    std::ofstream *outFD = args->outFD;
    int *frameID = args->frameID;
    gdouble threshold = args->threshold;
    
    auto buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    
    if (buffer == NULL)
        return GST_PAD_PROBE_OK;
    
    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (!caps)
        throw std::runtime_error("Can't get current caps");

    GVA::VideoFrame video_frame(buffer, caps);
    gint width = video_frame.video_info()->width;
    gint height = video_frame.video_info()->height;
    
    // Map buffer and create OpenCV image
    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ))
        return GST_PAD_PROBE_OK;    
    cv::Mat mat(height, width, CV_8UC4, map.data);
    
    // Parse and draw outputs
    std::vector<DetectionObject> objects;
    for (GVA::Tensor &tensor : video_frame.tensors()) {
        if (tensor.model_name().find("yolo_v3") != std::string::npos) {
            ParseYOLOV3Output(tensor, width, height, objects, threshold);
        }
    }
    
    DrawObjects(objects, mat, converter, outFD, args->video_id, frameID[0], args->streamVid, args->noDisplay, args->objIDArr);
    GST_PAD_PROBE_INFO_DATA(info) = buffer;
    
    gst_buffer_unmap(buffer, &map);
    gst_caps_unref(caps);
    frameID[0]++;
    return GST_PAD_PROBE_OK;
}


int parse_args(
    int argc, 
    char* argv[], 
    GOptionEntry opt_entries[],
    gchar** metaDataType,
    gchar** input_file,
    const std::string env_models_path, 
    gchar const** detection_model,
    gboolean* streamVid,
    gchar const** classification_models,
    const std::vector<std::string> default_classification_model_names,
    gchar const** model_precision) {
        
    // Parse arguments
    GOptionContext *context = g_option_context_new(" ");
    g_option_context_add_main_entries(context, opt_entries, " ");
    g_option_context_add_group(context, gst_init_get_option_group());
    GError *error = NULL;
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("option parsing failed: %s\n", error->message);
        return 1;
    }
    if (!input_file[0]) {
        g_print("Please specify input file:\n%s\n", g_option_context_get_help(context, TRUE, NULL));
        return 1;
    }
    
    if(!metaDataType[0]) {
        g_print("Please Specify metadata to query for:\n%s\n", g_option_context_get_help(context, TRUE, NULL));
        return 1;
    }
    
    if(strcmp(metaDataType[0], "object") != 0 && strcmp(metaDataType[0], "face") != 0) {
        g_print("Incorrect type of metadata to query for:\n%s\n", g_option_context_get_help(context, TRUE, NULL));
        return 1;
    }

    int VDMS_MAX_FILESIZE = 32 * 1024 * 1024;
    std::string input_file_str(input_file[0]);
    std::ifstream file(input_file_str, std::ios::binary | std::ios::ate);
    int fileSize = file.tellg();
    if(streamVid[0] && fileSize > VDMS_MAX_FILESIZE) {
        g_print("Video is too large to upload rerun without -v option:\n%s\n", g_option_context_get_help(context, TRUE, NULL));
        return -1;
    }
    
    if (env_models_path.empty()) 
        throw std::runtime_error("Enviroment variable MODELS_PATH is not set");
    
    if(!detection_model[0]) {
        if(strcmp(metaDataType[0], "object") == 0){
            std::vector<std::string> default_detection_model_names = {"yolo_v3.xml"};
            std::map<std::string, std::string> model_paths =
                FindModels(SplitString(env_models_path), default_detection_model_names, model_precision[0]);
            detection_model[0] = g_strdup(model_paths["yolo_v3.xml"].c_str());
        }
        else if(strcmp(metaDataType[0], "face") == 0){
            std::vector<std::string> default_detection_model_names = {"face-detection-adas-0001.xml"};
            std::map<std::string, std::string> model_paths =
            FindModels(SplitString(env_models_path), default_detection_model_names, model_precision[0]);
            detection_model[0] = g_strdup(model_paths["face-detection-adas-0001.xml"].c_str());
        }
    }
    
    if(classification_models[0] == NULL) {
        std::map<std::string, std::string> model_paths =
            FindModels(SplitString(env_models_path), default_classification_model_names, model_precision[0]);
        std::string classification_models_str = model_paths["age-gender-recognition-retail-0013.xml"] + "," +
                                                model_paths["emotions-recognition-retail-0003.xml"];
        classification_models[0] = g_strdup(classification_models_str.c_str());
    }    
    return 0;
}

int main(int argc, char *argv[]) {
    std::string env_models_path = "";
    const char *temp_model = getenv("MODELS_PATH");
    const char *temp_cvsdk = getenv("INTEL_CVSDK_DIR");
    if(temp_model != NULL){
        env_models_path = std::string() + temp_model;
    }
    else if(temp_cvsdk != NULL){
        env_models_path = std::string() + temp_cvsdk + "/deployment_tools/intel_models/";
    }
    
    const std::vector<std::string> default_classification_model_names = {
        "age-gender-recognition-retail-0013.xml", "emotions-recognition-retail-0003.xml"
    };
    gchar const *detection_model = NULL;
    gchar const *classification_models = NULL;
    gchar const *classification_model_1 = NULL;
    gchar const *classification_model_2 = NULL;
    gchar* input_file = NULL;
    gchar* extension = NULL;
    gchar* metaDataType = NULL;
    gchar const *device = "CPU";
    gchar const *model_precision = "FP32";
    gint batch_size = 1;
    gdouble threshold = 0.7;
    gboolean no_display = FALSE;
    int frameID = 0;
    gchar* vdms_ip_addr = (gchar*)"localhost";
    gchar* outFN = NULL;
    gboolean streamVid = FALSE;
    gboolean logVDMSTransX = FALSE;
    gchar* objIDListFN = NULL;
    
    static GOptionEntry opt_entries[] = {
        {"input_file", 'i', 0, G_OPTION_ARG_STRING, &input_file, "Path to input video file", NULL},
        {"precision", 'p', 0, G_OPTION_ARG_STRING, &model_precision, "Models precision. Default: FP32", NULL},
        {"detection", 'm', 0, G_OPTION_ARG_STRING, &detection_model, "Path to detection model file", NULL},
        {"classification", 'c', 0, G_OPTION_ARG_STRING, &classification_models, "Path to classification models as ',' separated list", NULL},        
        {"extension", 'e', 0, G_OPTION_ARG_STRING, &extension, "Path to custom layers extension library", NULL},
        {"device", 'd', 0, G_OPTION_ARG_STRING, &device, "Device to run inference", NULL},
        {"batch", 'b', 0, G_OPTION_ARG_INT, &batch_size, "Batch size", NULL},
        {"threshold", 't', 0, G_OPTION_ARG_DOUBLE, &threshold, "Confidence threshold for detection (0 - 1)", NULL},
        {"no-display", 'n', 0, G_OPTION_ARG_NONE, &no_display, "Run without display", NULL},
        {"vdms_ip_addr", 'a', 0, G_OPTION_ARG_STRING, &vdms_ip_addr, "IP address of the VDMS Server", NULL},
        {"outFN", 'f', 0, G_OPTION_ARG_STRING, &outFN, "File to print MetaData to", NULL},
        {"streamVid", 'v', 0, G_OPTION_ARG_NONE, &streamVid, "Flag to upload video to the VDMS Server", NULL},
        {"objIDListFN", 'o', 0, G_OPTION_ARG_STRING, &objIDListFN, "File which contains a list of Obj ids to filter on. For full list see metaData_extract.hpp", NULL},
        {"metaDataType", 'x', 0, G_OPTION_ARG_STRING, &metaDataType, "Type of MetaData extraction to run [object | face]", NULL},
        {"logVDMSTransX", 'l', 0, G_OPTION_ARG_NONE, &logVDMSTransX, "Flag to dump VDMS transactions to the console", NULL},
        {}
    };
      
    if(parse_args(
        argc, 
        argv, 
        opt_entries,
        &metaDataType,
        &input_file,
        env_models_path,
        &detection_model,
        &streamVid,
        &classification_models,
        default_classification_model_names,
        &model_precision) != 0) {
        return 1;
    }
        
    
    // Get classification models
    std::stringstream ss(classification_models);
    std::vector<std::string> res;
    while(ss.good()) {
        std::string substr;
        getline(ss, substr, ',');
        res.push_back(substr);
    }
    classification_model_1 = res[0].c_str();
    classification_model_2 = res[1].c_str();

    std::vector<std::string> objIDArr = VDMSConverter::crtObjIDArr(objIDListFN, coco_labels);
    if(objIDArr.empty()) 
    {
        std::cout << "Object ID List is empty" << std::endl;
        return 1;
    }
    VDMSConverter *converter = NULL;
    std::string vdms_ip_addr_str(vdms_ip_addr);
    converter = new VDMSConverter(vdms_ip_addr_str, (bool)logVDMSTransX);
    std::string input_file_str(input_file);
    if(converter->upload_video(input_file_str, streamVid) == -1) 
    {
        std::cout << "Add Video failed" << std::endl;
        return 1;
    } 
    
    // set probe callback
    pad_probe_CB_args args;
    std::ofstream *outFD = (outFN != NULL) ? new std::ofstream(outFN) : NULL;
    args.converter = converter;
    args.outFD = outFD;
    args.video_id = input_file;
    args.threshold = threshold;
    args.frameID = &frameID;
    args.streamVid = (bool)streamVid;
    args.noDisplay = (bool)no_display;
    args.objIDArr = objIDArr;
    
    // Build the pipeline
    GstElement *gstBin = NULL;
    GstPad *pad = NULL;
    GstElement *pipeline = NULL;
    
    if(strcmp(metaDataType, "object") == 0)
    {
        gchar const* preprocess_pipeline = "decodebin ! videoconvert n-threads=4 ! videoscale n-threads=4 ";
        gchar const* capfilter = "video/x-raw,format=BGRA";
        gchar const* sink = no_display ? "identity signal-handoffs=false ! fakesink sync=false"
                                       : "autovideosink";

        gchar* launch_str = g_strdup_printf("filesrc location=%s ! %s ! capsfilter caps=\"%s\" ! "
                            "gvainference name=gvadetect model=%s device=%s batch-size=%d ! queue ! "
                            "videoconvert n-threads=4 ! %s ",
                            input_file, preprocess_pipeline, capfilter, detection_model, device, batch_size, sink);
        
        g_print("PIPELINE: %s \n", launch_str);
        pipeline = gst_parse_launch(launch_str, NULL);
        g_free(launch_str); 
        
        // Set Probe callback
        gstBin = gst_bin_get_by_name(GST_BIN(pipeline), "gvadetect");
        pad = gst_element_get_static_pad(gstBin, "src");
        gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, pad_probe_callback_obj, &args, NULL);
    }
    else if(strcmp(metaDataType, "face") == 0){
        gchar const* preprocess_pipeline = "decodebin ! videoconvert n-threads=4 ! videoscale n-threads=4 ";
        gchar const* capfilter = "video/x-raw,format=BGRA";
        gchar const *sink = no_display ? "identity signal-handoffs=false ! fakesink sync=false"
                               : "autovideosink";

        gchar* launch_str = g_strdup_printf("filesrc location=%s ! %s ! capsfilter caps=\"%s\" ! "
                                          "gvadetect model=%s device=%s batch-size=%d ! queue ! "
                                          "gvaclassify  model=%s device=%s batch-size=%d ! queue ! "
                                          "gvaclassify  model=%s device=%s batch-size=%d ! queue ! "
                                          "gvawatermark name=gvawatermark ! videoconvert n-threads=4 ! %s",
                                          input_file, preprocess_pipeline, capfilter,
                                          detection_model, device, batch_size,
                                          classification_model_1, device, batch_size,
                                          classification_model_2, device, batch_size, sink);
        // }
        
        g_print("PIPELINE: %s \n", launch_str);
        pipeline = gst_parse_launch(launch_str, NULL);
        g_free(launch_str);
        
        // set probe callback
        gstBin = gst_bin_get_by_name(GST_BIN(pipeline), "gvawatermark");
        pad = gst_element_get_static_pad(gstBin, "src");
        gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, pad_probe_callback_face, &args, NULL);
    }
    gst_object_unref(pad);
    
    // Start playing
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    
    // Wait until error or EOS
    GstBus *bus = gst_element_get_bus(pipeline);    
    int ret_code = 0;    
    GstMessage *msg = gst_bus_poll(bus, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS), -1);    
    if (msg && GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
        GError *err = NULL;
        gchar *dbg_info = NULL;        
        gst_message_parse_error(msg, &err, &dbg_info);
        g_printerr("ERROR from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
        g_printerr("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");        
        g_error_free(err);
        g_free(dbg_info);
        ret_code = -1;
    }
    
    if (msg)
        gst_message_unref(msg);
    
    // Free resources
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    
    if(outFD)
        outFD->close();
    
    return ret_code;
}
