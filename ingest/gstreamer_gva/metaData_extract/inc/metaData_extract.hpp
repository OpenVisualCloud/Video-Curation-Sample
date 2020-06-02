// --------------------------------------------------------------------------------------------------------------------------------------------------
// Author: Ikenna Okafor (iokafor)
// --------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once


// Definitions
#define UNUSED(x) (void)(x)


// System Includes
#include <algorithm>
#include <dirent.h>
#include <gio/gio.h>
#include <gst/gst.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string>


// Project Includes
#include "tensor.h"
#include "draw_axes.h"
#include "metadata/gva_tensor_meta.h"
#include "video_frame.h"
#include "vdms_converter.hpp"


std::vector<std::string> coco_labels = {
    "person",         "bicycle",    "car",           "motorbike",     "aeroplane",   "bus",           "train",
    "truck",          "boat",       "traffic light", "fire hydrant",  "stop sign",   "parking meter", "bench",
    "bird",           "cat",        "dog",           "horse",         "sheep",       "cow",           "elephant",
    "bear",           "zebra",      "giraffe",       "backpack",      "umbrella",    "handbag",       "tie",
    "suitcase",       "frisbee",    "skis",          "snowboard",     "sports ball", "kite",          "baseball bat",
    "baseball glove", "skateboard", "surfboard",     "tennis racket", "bottle",      "wine glass",    "cup",
    "fork",           "knife",      "spoon",         "bowl",          "banana",      "apple",         "sandwich",
    "orange",         "broccoli",   "carrot",        "hot dog",       "pizza",       "donut",         "cake",
    "chair",          "sofa",       "pottedplant",   "bed",           "diningtable", "toilet",        "tvmonitor",
    "laptop",         "mouse",      "remote",        "keyboard",      "cell phone",  "microwave",     "oven",
    "toaster",        "sink",       "refrigerator",  "book",          "clock",       "vase",          "scissors",
    "teddy bear",     "hair drier", "toothbrush"
};


class pad_probe_CB_args 
{
    public:
        VDMSConverter *converter;
        std::ofstream *outFD;
        int *frameID;
        std::string video_id;
        gdouble threshold;
        bool streamVid;
        std::vector<std::string> objIDArr;
        bool noDisplay;
};


class DetectionObject 
{
    public:
        DetectionObject(double x, double y, double h, double w, int class_id, float confidence, float h_scale,
                        float w_scale
        ) {
            this->xmin = static_cast<int>((x - w / 2) * w_scale);
            this->ymin = static_cast<int>((y - h / 2) * h_scale);
            this->xmax = static_cast<int>(this->xmin + w * w_scale);
            this->ymax = static_cast<int>(this->ymin + h * h_scale);
            this->class_id = class_id;
            this->confidence = confidence;
        }
        ~DetectionObject() {}
        bool operator < (const DetectionObject &s2) const {
            return this->confidence < s2.confidence;
        }
        int xmin;
        int ymin;
        int xmax;
        int ymax;
        int class_id;
        float confidence;
};


void DrawObjects(std::vector<DetectionObject> &objects, cv::Mat &frame, VDMSConverter *converter, std::ofstream *outFD, std::string video_id, int frameID, bool streamVid, bool noDisplay, std::vector<std::string> objFilArr);


static int EntryIndex(int side, int lcoords, int lclasses, int location, int entry);


void ExploreDir(std::string search_dir, const std::string &model_name, std::vector<std::string> &result);


std::vector<std::string> FindModel(const std::vector<std::string> &search_dirs, const std::string &model_name);


std::map<std::string, std::string> FindModels(
    const std::vector<std::string> &search_dirs,
    const std::vector<std::string> &model_names,
    const std::string &precision
);


double IntersectionOverUnion(const DetectionObject &box_1, const DetectionObject &box_2);


static GstPadProbeReturn pad_probe_callback_obj(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);


static GstPadProbeReturn pad_probe_callback_face(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

int parse_args(
    int argc, 
    char* argv[], 
    GOptionEntry opt_entries[],
    gchar const *metaDataType,
    gchar const *input_file,
    const std::string env_models_path, 
    gchar const *detection_model,
    gboolean streamVid,
    gchar const *classification_models,
    const std::vector<std::string> default_classification_model_names,
    gchar const *model_precision
);

void ParseYOLOV3Output(
    GVA::Tensor &tensor_yolo, 
    int image_width, 
    int image_height,
    std::vector<DetectionObject> &objects, 
    gdouble threshold
);


std::vector<std::string> SplitString(const std::string input, char delimiter = ':');


std::string to_upper_case(std::string str);
