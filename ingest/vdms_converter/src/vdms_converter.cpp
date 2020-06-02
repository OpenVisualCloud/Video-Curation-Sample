// Project Includes
#include "vdms_converter.hpp"

VDMSConverter::VDMSConverter(std::string& server_address, bool dumpTransX)
{
    _aclient.reset(new VDMS::VDMSClient(server_address));
    std::cout << "Connected to VDMS Server" << std::endl;
    _numTransX = 0;
    _dumpTransX = dumpTransX;
}


VDMSConverter::~VDMSConverter()
{

}


void VDMSConverter::add_bbox_props(int age, bbox_obj& bbox, const std::string& emotion, int frameID, const std::string& gender)
{
    bbox.props[VDMS_OBJID] = VDMS_OBJID_FACE;
    bbox.props[VDMS_AGE] = age;
    bbox.props[VDMS_FRAMEID] = frameID;
    if(emotion == "neutral")
    {
        bbox.props[VDMS_EMOT] = VDMS_EMOT_NEUTRAL;
    }
    else if(emotion == "happy")
    {
        bbox.props[VDMS_EMOT] = VDMS_EMOT_HAPPY;
    }
    else if(emotion == "sad")
    {
        bbox.props[VDMS_EMOT] = VDMS_EMOT_SAD;
    }
    else if(emotion == "surprise")
    {
        bbox.props[VDMS_EMOT] = VDMS_EMOT_SURPRISE;
    }
    else if(emotion == "anger")
    {
        bbox.props[VDMS_EMOT] = VDMS_EMOT_ANGER;
    }
    if(gender == "male")
    {
        bbox.props[VDMS_GEND] = VDMS_GEND_MALE;
    }
    else if(gender == "female")
    {
        bbox.props[VDMS_GEND] = VDMS_GEND_FEMALE;
    }
}


std::vector<std::string> VDMSConverter::crtObjIDArr(char* fname, std::vector<std::string> coco_labels)
{
    std::vector<std::string> objIDArr;
    if(fname == NULL)
    {
        objIDArr = coco_labels;
        return objIDArr;
    }
    else
    {
        std::string fname_str(fname);
        std::ifstream fd(fname_str);
        if(!fd.good())
        {
            std::cout << fname_str << " does not exist" << std::endl;
            return std::vector<std::string>();
        }
        std::string objectID;
        while(std::getline(fd, objectID))
        {
            if(std::find(coco_labels.begin(), coco_labels.end(), objectID) != coco_labels.end())
            {
                objIDArr.push_back(objectID);
            }
            else
            {
                std::cout << objectID << " is not a valid Object ID. See coco_labels.h for valid objectID's" << std::endl;
            }
        }
        return objIDArr;
    }
}


double VDMSConverter::getVidFrameRate(std::string video_id)
{
    cv::VideoCapture cap(video_id);
    if(!cap.isOpened()) {
        std::cout << "Could not find " << video_id << std::endl;
        return -1.0f;
    }
    return cap.get(5);
}


std::vector<frameInfo_obj> VDMSConverter::find_emotion(const std::string& emotion, bool showBB, const std::string& video_id, bool streamVid)
{
    std::cout << "Querying VDMS Server for frames with " << emotion
              << " faces" << std::endl;
    Json::Value qry;
    int qry_ref = crte_find_emot_frmes_qry(emotion, qry, video_id, streamVid);
    if(qry.empty())
    {
        std::cout << "Creating Query Failed" << std::endl;
        return std::vector<frameInfo_obj>();
    }
    Json::Value parsed_json;
    VDMS::Response resp;
    if(send_qry(parsed_json, qry, resp))
    {
        std::cout << "Send Query Failed" << std::endl;
        return std::vector<frameInfo_obj>();
    }
    int qry_ofst = qry_ref - 2;
    int numBB = parsed_json[qry_ofst]["FindBoundingBox"]["returned"].asInt();
    if(numBB == 0)
    {
        std::cout << "No Bounding Boxes found" << std::endl;
    }
    else
    {
        double frameRate;
        std::vector<frameInfo_obj> frameInfoArr = post_find_qry(
            frameRate,
            numBB,
            parsed_json,
            qry_ofst,
            streamVid
        );
        if(frameInfoArr.size() != 0)
        {
            return frameInfoArr;
        }
    }
    
    return std::vector<frameInfo_obj>();
}


std::vector<frameInfo_obj> VDMSConverter::find_objects(std::vector<std::string> objIDArr, bool showBB, const std::string& video_id, bool streamVid)
{
    std::cout << "Querying VDMS Server for frames with: " << std::endl;
    for(int i = 0; i < objIDArr.size(); i++)
    {
        std::cout << objIDArr[i] << std::endl;
    }
    Json::Value qry;
    int qry_ref = crte_find_obj_frmes_qry(objIDArr, qry, video_id, streamVid);
    if(qry.empty())
    {
        std::cout << "Creating Query Failed" << std::endl;
        return std::vector<frameInfo_obj>();
    }
    Json::Value parsed_json;
    VDMS::Response resp;
    if(send_qry(parsed_json, qry, resp))
    {
        std::cout << "Send Query Failed" << std::endl;
        return std::vector<frameInfo_obj>();
    }
    int qry_ofst = qry_ref - 2;
    int numBB = parsed_json[qry_ofst]["FindBoundingBox"]["returned"].asInt();
    if(numBB == 0)
    {
        std::cout << "No Bounding Boxes found" << std::endl;
    }
    else
    {
        double frameRate;
        std::vector<frameInfo_obj> frameInfoArr = post_find_qry(
            frameRate,
            numBB,
            parsed_json,
            qry_ofst,
            streamVid
        );
        if(frameInfoArr.size() != 0)
        {
            return frameInfoArr;
        }
    }
    return std::vector<frameInfo_obj>();
}


std::vector<frameInfo_obj> VDMSConverter::find_people(int n, bool numPPL_exact, bool showBB, const std::string& video_id, bool streamVid)
{
    if(numPPL_exact)
    {
        std::cout << "Querying VDMS Server for frames with " << n
                  << " people" << std::endl;
    }
    else
    {
        std::cout << "Querying VDMS Server for frames with people in video: " << video_id << std::endl;
    }
    Json::Value qry;
    int qry_ref = crte_find_vid_frmes_ent_qry(qry, video_id, streamVid);
    if(qry.empty())
    {
        std::cout << "Creating Query Failed" << std::endl;
        return std::vector<frameInfo_obj>();
    }
    Json::Value parsed_json;
    VDMS::Response resp;
    if(send_qry(parsed_json, qry, resp))
    {
        std::cout << "Send Query Failed" << std::endl;
        return std::vector<frameInfo_obj>();
    }
    int qry_ofst = qry_ref - 2;
    int numFrames = parsed_json[qry_ofst]["FindEntity"]["returned"].asInt();
    std::vector<frameInfo_obj> frameInfoArr;
    if(numFrames == 0)
    {
        std::cout << "No Frames found" << std::endl;
    }
    else
    {
        qry.clear();
        qry_ref = crte_find_ppl_frmes_qry(
            numFrames,
            parsed_json,
            qry,
            qry_ofst,
            video_id,
            streamVid
        );
        if(qry.empty())
        {
            std::cout << "Creating Query Failed" << std::endl;
            return std::vector<frameInfo_obj>();
        }
        if(send_qry(parsed_json, qry, resp))
        {
            std::cout << "Send Query Failed" << std::endl;
            return std::vector<frameInfo_obj>();
        }
        qry_ofst = 1;
        double frameRate;
        std::vector<frameInfo_obj> frameInfoArr = post_find_ppl_qry(
            frameRate,
            n,
            numPPL_exact,
            numFrames,
            parsed_json,
            qry_ofst,
            streamVid
        );
        if(frameInfoArr.size() != 0)
        {
            return frameInfoArr;
        }
    }

    return std::vector<frameInfo_obj>();
}


int VDMSConverter::upload_data(const bbox_obj& bbox, int frameID, const std::string& video_id, bool streamVid)
{
    Json::Value qry;
    crte_find_frame_ent_qry(frameID, qry, video_id, streamVid);

    if(qry.empty())
    {
        return -1;
    }
    Json::Value parsed_json;
    VDMS::Response resp;
    bool newFrame = false;

    int result =send_qry(parsed_json, qry, resp);
    if(result == -1)
    {
        newFrame = true;
    }
    qry.clear();
    crte_add_bbox_qry(bbox, frameID, newFrame, qry, video_id, streamVid);
    return send_qry(parsed_json, qry, resp);
}


int VDMSConverter::upload_video(const std::string& video_id, bool streamVid)
{
    std::ifstream fd(video_id);
    if(!fd.good())
    {
        std::cout << video_id << " does not exist" << std::endl;
        return -1;
    }
    Json::Value qry;
    crte_upld_vid_qry(qry, video_id, streamVid);
    if(qry.empty())
    {
        return - 1;
    }
    Json::Value parsed_json;
    VDMS::Response resp;
    int retV = send_qry(parsed_json, qry, resp);
    return post_upld_vid_qry(retV, video_id, streamVid);
}


int VDMSConverter::add_bbox(const bbox_obj& bbox, Json::Value& qry, int qry_ref)
{
    Json::Value bounding_box;
    bounding_box["AddBoundingBox"]["_ref"] = qry_ref;
    bounding_box["AddBoundingBox"]["rectangle"]["x"] = (bbox.x < 0) ? 0 : bbox.x;
    bounding_box["AddBoundingBox"]["rectangle"]["y"] = (bbox.y < 0) ? 0 : bbox.y;
    bounding_box["AddBoundingBox"]["rectangle"]["w"] = bbox.w;
    bounding_box["AddBoundingBox"]["rectangle"]["h"] = bbox.h;
    add_props(bounding_box["AddBoundingBox"], bbox.props);
    qry.append(bounding_box);
    return bounding_box["AddBoundingBox"]["_ref"].asInt();
}


int VDMSConverter::add_connection(const std::string& edgeClass, int node_ref1, int node_ref2, Json::Value& qry, const Json::Value& props)
{
    Json::Value connection;
    connection["AddConnection"]["ref1"] = node_ref1;
    connection["AddConnection"]["ref2"] = node_ref2;
    connection["AddConnection"]["class"] = edgeClass;
    add_props(connection["AddConnection"], props);
    qry.append(connection);
    return connection["AddConnection"]["_ref"].asInt();
}


void VDMSConverter::add_constraints(Json::Value& json_node, const Json::Value& constraints)
{
    if(constraints.size() >= 1)
    {
        json_node["constraints"] = constraints;
    }
}


int VDMSConverter::add_frame_entity(int frameID, Json::Value& qry, int qry_ref, const std::string& video_id)
{
    Json::Value frame_entity;
    frame_entity["AddEntity"]["_ref"] = qry_ref;
    frame_entity["AddEntity"]["class"] = VDMS_CLASS_FRAME;
    frame_entity["AddEntity"]["properties"][VDMS_FRAMEID] = frameID;
    qry.append(frame_entity);
    return frame_entity["AddEntity"]["_ref"].asInt();
}


void VDMSConverter::add_props(Json::Value& json_node, const Json::Value& props)
{
    if(props.size() >= 1)
    {
        json_node["properties"] = props;
    }
}


int VDMSConverter::add_video(Json::Value& qry, int qry_ref, const std::string& video_id, const Json::Value& props)
{
    Json::Value video;
    std::string ext = getExtenstion(video_id);
    video["AddVideo"]["_ref"] = qry_ref;
    video["AddVideo"]["container"] = ext;
    add_props(video["AddVideo"], props);
    qry.append(video);
    return video["AddVideo"]["_ref"].asInt();
}


int VDMSConverter::add_video_entity(Json::Value& qry, int qry_ref, const std::string& video_id, const Json::Value& props)
{
    Json::Value video;
    video["AddEntity"]["_ref"] = qry_ref;
    video["AddEntity"]["class"] = VDMS_CLASS_VIDEO;
    add_props(video["AddEntity"], props);
    Json::Value constraints;
    constraints[VDMS_VIDNAME][0] = "==";
    constraints[VDMS_VIDNAME][1] = video_id;
    add_constraints(video["AddEntity"], constraints);
    qry.append(video);
    return video["AddEntity"]["_ref"].asInt();
}


int VDMSConverter::crte_add_bbox_qry(const bbox_obj& bbox, int frameID, bool newFrame, Json::Value& qry, const std::string& video_id, bool streamVid)
{
    int qry_ref = 1;
    Json::Value vid_constr;
    vid_constr[VDMS_VIDNAME][0] = "==";
    vid_constr[VDMS_VIDNAME][1] = video_id;
    int video_ref = find_video(qry, qry_ref, vid_constr, streamVid);
    qry_ref++;
    int frame_ref;
    if(newFrame)
    {
        frame_ref = add_frame_entity(frameID, qry, qry_ref, video_id);
        qry_ref++;
        Json::Value vid2frame_props;
        vid2frame_props["video_name"] = video_id;
        vid2frame_props[VDMS_FRAMEID] = frameID;
        add_connection(VDMS_EDGE_CLASS_VID2FRAME, video_ref, frame_ref, qry, vid2frame_props);
    }
    else
    {
        Json::Value frme_constr;
        frme_constr[VDMS_FRAMEID][0] = "==";
        frme_constr[VDMS_FRAMEID][1] = frameID;
        frame_ref = find_frame_entity(video_ref, qry, qry_ref, frme_constr);
        qry_ref++;
    }
    int bbox_ref = add_bbox(bbox, qry, qry_ref);
    Json::Value frame2bb_props;
    frame2bb_props["video_name"] = video_id;
    frame2bb_props[VDMS_FRAMEID] = frameID;
    add_connection(VDMS_EDGE_CLASS_FRME2BB, frame_ref, bbox_ref, qry, frame2bb_props);
    return qry_ref;
}


int VDMSConverter::crte_find_emot_frmes_qry(std::string emotion, Json::Value& qry, const std::string& video_id, bool streamVid)
{
    int qry_ref = 1;
    Json::Value vid_constr;
    vid_constr[VDMS_VIDNAME][0] = "==";
    vid_constr[VDMS_VIDNAME][1] = video_id;
    Json::Value vid_props;
    vid_props[VDMS_VIDFRMRATE];
    int video_ref = find_video(qry, qry_ref, vid_constr, streamVid, vid_props);
    qry_ref++;
    int frame_ref = find_frame_entity(video_ref, qry, qry_ref);
    qry_ref++;
    Json::Value bbox_constr;
    bbox_constr[VDMS_OBJID][0] = "==";
    bbox_constr[VDMS_OBJID][1] = VDMS_OBJID_FACE;
    bbox_constr[VDMS_EMOT][0] = "==";
    bbox_constr[VDMS_EMOT][1] = emotion;
    Json::Value bbox_props;
    bbox_props[VDMS_FRAMEID];
    bbox_props["_coordinates"];
    Json::Value bbox_res;
    bbox_res["sort"] = VDMS_FRAMEID;
    find_bbox(frame_ref, qry, qry_ref, bbox_constr, bbox_props, bbox_res);
    qry_ref++;
    return qry_ref;
}


int VDMSConverter::crte_find_obj_frmes_qry(std::vector<std::string> objIDArr, Json::Value& qry, const std::string& video_id, bool streamVid)
{
    int qry_ref = 1;
    Json::Value vid_constr;
    vid_constr[VDMS_VIDNAME][0] = "==";
    vid_constr[VDMS_VIDNAME][1] = video_id;
    Json::Value vid_props;
    vid_props[VDMS_VIDFRMRATE];
    int video_ref = find_video(qry, qry_ref, vid_constr, streamVid, vid_props);
    qry_ref++;
    int frame_ref = find_frame_entity(video_ref, qry, qry_ref);
    qry_ref++;
    Json::Value bbox_constr;
    for(int i = 0; i < objIDArr.size(); i++)
    {
        bbox_constr[VDMS_OBJID][0] = "==";
        bbox_constr[VDMS_OBJID][1] = objIDArr[i];
    }
    Json::Value bbox_props;
    bbox_props[VDMS_FRAMEID];
    bbox_props["_coordinates"];
    Json::Value bbox_res;
    bbox_res["sort"] = VDMS_FRAMEID;
    find_bbox(frame_ref, qry, qry_ref, bbox_constr, bbox_props, bbox_res);
    qry_ref++;
    return qry_ref;
}


void VDMSConverter::crte_find_frame_ent_qry(int frameID, Json::Value& qry, const std::string& video_id, bool streamVid)
{
    int qry_ref = 1;
    Json::Value vid_constr;
    vid_constr[VDMS_VIDNAME][0] = "==";
    vid_constr[VDMS_VIDNAME][1] = video_id;
    int video_ref = find_video(qry, qry_ref, vid_constr, streamVid);
    qry_ref++;
    Json::Value frme_constr;
    frme_constr[VDMS_FRAMEID][0] = "==";
    frme_constr[VDMS_FRAMEID][1] = frameID;
    find_frame_entity(video_ref, qry, qry_ref, frme_constr);
    qry_ref++;
}


int VDMSConverter::crte_find_vid_frmes_ent_qry(Json::Value& qry, const std::string& video_id, bool streamVid)
{
    int qry_ref = 1;
    Json::Value constraints;
    constraints[VDMS_VIDNAME][0] = "==";
    constraints[VDMS_VIDNAME][1] = video_id;
    int video_ref = find_video(qry, qry_ref, constraints, streamVid);
    qry_ref++;
    Json::Value props;
    props[VDMS_FRAMEID];
    Json::Value res;
    res["sort"] = VDMS_FRAMEID;
    findConnectedEntity(VDMS_EDGE_CLASS_VID2FRAME, VDMS_CLASS_FRAME, video_ref, qry, qry_ref, props, res);
    qry_ref++;
    return qry_ref;
}


int VDMSConverter::crte_find_ppl_frmes_qry(
    int numFrames,
    Json::Value parsed_json,
    Json::Value& qry,
    int qry_ofst,
    const std::string& video_id,
    bool streamVid
) {
    int qry_ref = 1;
    Json::Value vid_constr;
    vid_constr[VDMS_VIDNAME][0] = "==";
    vid_constr[VDMS_VIDNAME][1] = video_id;
    Json::Value vid_props;
    vid_props[VDMS_VIDFRMRATE];
    int video_ref = find_video(qry, qry_ref, vid_constr, streamVid, vid_props);
    qry_ref++;
    Json::Value bbox_res;
    for(int i = 0; i < numFrames; i++)
    {
        int frameID = parsed_json[qry_ofst]["FindEntity"]["entities"][i][VDMS_FRAMEID].asInt();
        Json::Value frme_constr;
        frme_constr[VDMS_FRAMEID][0] = "==";
        frme_constr[VDMS_FRAMEID][1] = frameID;
        Json::Value frame_props;
        frame_props[VDMS_FRAMEID];
        int frame_ref = find_frame_entity(video_ref, qry, qry_ref, frme_constr, frame_props);
        qry_ref++;
        Json::Value bbox_props;
        bbox_props["_coordinates"];
        bbox_props[VDMS_OBJID];
        find_bbox(frame_ref, qry, qry_ref, Json::Value(), bbox_props);
        qry_ref++;
    }
    return qry_ref;
}


int VDMSConverter::crte_upld_vid_qry(Json::Value& qry, const std::string& video_path, bool streamVid)
{
    int qry_ref = 1;
    Json::Value props;
    props[VDMS_VIDNAME] = video_path;
    double frameRate = getVidFrameRate(video_path);
    if(frameRate == -1.0f)
    {
        return -1;
    }
    props[VDMS_VIDFRMRATE] = frameRate;
    if(streamVid)
    {
        Json::Value constraints;
        constraints[VDMS_VIDNAME][0] = "==";
        constraints[VDMS_VIDNAME][1] = video_path;
        find_video(qry, qry_ref, constraints, Json::Value());
        qry_ref++;
    }
    else
    {

        add_video_entity(qry, qry_ref, video_path, props);
        qry_ref++;
    }
    return qry_ref;
}


void VDMSConverter::deserialFrames(std::vector<frameInfo_obj>& frameInfoArr, int ofst, const VDMS::Response& resp)
{
    for(int i = 0; i < resp.blobs.size(); i++)
    {
        std::vector<char> byteStream;
        const char *byte_ptr = resp.blobs[i].c_str();
        for(int j = 0; j < resp.blobs[i].length(); j++)
        {
            byteStream.push_back(byte_ptr[j]);
        }
        frameInfoArr[i + ofst].frame = cv::imdecode(cv::Mat(byteStream), cv::IMREAD_COLOR);
        byteStream.clear();
    }
}


int VDMSConverter::downloadFrames(std::vector<frameInfo_obj>& frameInfoArr, const std::string& video_id)
{
    std::cout << "Downloading Frames from VDMS Server" << std::endl;
    int numQrys = ceil(float(frameInfoArr.size()) / float(VDMS_TX_FRAME_BATCH_SIZE));
    int rem_frames = frameInfoArr.size();
    int ofst = 0;
    for(int i = 0; i < numQrys; i++)
    {
        std::vector<int> frameIDArr;
        int numFramesReqd = std::min(VDMS_TX_FRAME_BATCH_SIZE, rem_frames);
        for(int j = 0; j < numFramesReqd; j++)
        {
            frameIDArr.push_back(frameInfoArr[j + ofst].frameID);
        }
        Json::Value qry;
        int qry_ref = 1;
        find_frames(frameIDArr, qry, qry_ref, video_id);
        VDMS::Response resp;
        Json::Value parsed_json;
        if(send_qry(parsed_json, qry, resp) == -1)
        {
            std::cout << "Send Query Failed" << std::endl;
            return -1;
        }
        deserialFrames(frameInfoArr, ofst, resp);
        rem_frames -= numFramesReqd;
        frameIDArr.clear();
        ofst += numFramesReqd;
    }
    return 0;
}


void VDMSConverter::find_bbox(
    int link_ref,
    Json::Value& qry,
    int& qry_ref,
    const Json::Value& constraints,
    const Json::Value& props,
    const Json::Value& res
) {
    Json::Value bbox_json;
    bbox_json["FindBoundingBox"]["_ref"] = qry_ref;
    bbox_json["FindBoundingBox"]["link"]["ref"] = link_ref;
    bbox_json["FindBoundingBox"]["link"]["class"] = VDMS_EDGE_CLASS_FRME2BB;
    get_props(bbox_json["FindBoundingBox"], props);
    add_constraints(bbox_json["FindBoundingBox"], constraints);
    set_results(bbox_json["FindBoundingBox"], res);
    qry.append(bbox_json);
}


void VDMSConverter::findConnectedEntity(
    std::string edge_class,
    std::string entity_class,
    int link_ref,
    Json::Value& qry,
    int qry_ref,
    const Json::Value& props,
    const Json::Value& res
) {
    Json::Value connectedEntity;
    connectedEntity["FindEntity"]["_ref"] = qry_ref;
    connectedEntity["FindEntity"]["class"] = entity_class;
    connectedEntity["FindEntity"]["link"]["ref"] = link_ref;
    connectedEntity["FindEntity"]["link"]["class"] = edge_class;
    get_props(connectedEntity["FindEntity"], props);
    set_results(connectedEntity["FindEntity"], res);
    qry.append(connectedEntity);
}


int VDMSConverter::find_frames(const std::vector<int>& frameIDArr, Json::Value& qry, int qry_ref, const std::string& video_id)
{
    Json::Value frame;
    frame["FindFrames"]["_ref"] = qry_ref;
    for(int i = 0; i < frameIDArr.size(); i++)
    {
        frame["FindFrames"]["frames"][i] = frameIDArr[i];
    }
    Json::Value constraints;
    constraints[VDMS_VIDNAME][0] = "==";
    constraints[VDMS_VIDNAME][1] = video_id;
    add_constraints(frame["FindFrames"], constraints);
    qry.append(frame);
    return frame["FindFrames"]["_ref"].asInt();
}


int VDMSConverter::find_frame_entity(
    int link_ref,
    Json::Value& qry,
    int qry_ref,
    const Json::Value& constraints,
    const Json::Value& props,
    const Json::Value& res
) {
    Json::Value frame;
    frame["FindEntity"]["_ref"] = qry_ref;
    frame["FindEntity"]["class"] = VDMS_CLASS_FRAME;
    frame["FindEntity"]["link"]["ref"] = link_ref;
    add_constraints(frame["FindEntity"], constraints);
    get_props(frame["FindEntity"], props);
    set_results(frame["FindEntity"], res);
    qry.append(frame);
    return frame["FindEntity"]["_ref"].asInt();
}


int VDMSConverter::find_video(
    Json::Value& qry,
    int qry_ref,
    const Json::Value& constraints,
    bool streamVid,
    const Json::Value& props
) {
    if(streamVid)
    {
        return find_video(qry, qry_ref, constraints, props);
    }
    else
    {
        return find_video_entity(qry, qry_ref, constraints, props);
    }
}


int VDMSConverter::find_video(
    Json::Value& qry,
    int qry_ref,
    const Json::Value& constraints,
    const Json::Value& props
) {
    Json::Value video;
    video["FindVideo"]["_ref"] = qry_ref;
    add_constraints(video["FindVideo"], constraints);
    get_props(video["FindVideo"], props);
    qry.append(video);
    return video["FindVideo"]["_ref"].asInt();
}


int VDMSConverter::find_video_entity(
    Json::Value& qry,
    int qry_ref,
    const Json::Value& constraints,
    const Json::Value& props
) {
    Json::Value video;
    video["FindEntity"]["_ref"] = qry_ref;
    video["FindEntity"]["class"] = VDMS_CLASS_VIDEO;
    add_constraints(video["FindEntity"], constraints);
    get_props(video["FindEntity"], props);
    qry.append(video);
    return video["FindEntity"]["_ref"].asInt();
}


std::string VDMSConverter::getExtenstion(const std::string& video_id)
{
    int dotIdx = video_id.find_last_of(".");
    int len = video_id.length() - dotIdx;
    return video_id.substr(dotIdx + 1, len);
}


void VDMSConverter::get_props(Json::Value& json_node, Json::Value props)
{
    Json::Value::Members propNameArr = props.getMemberNames();
    for(int i = 0; i < props.size(); i++)
    {
        std::string propName = propNameArr[i];
        json_node["results"]["list"][i] = propName;
        for(int j = 0; j < props[propName].size(); j++)
        {
            json_node["results"]["list"][i][propName][j] = props[propName][j];
        }
    }
}


void VDMSConverter::set_results(Json::Value& json_node, const Json::Value& res)
{
    Json::Value::Members resNameArr = res.getMemberNames();
    for(int i = 0; i < res.size(); i++)
    {
        std::string resName = resNameArr[i];
        json_node["results"][resName] = res[resName];
    }
}


double VDMSConverter::getVidFrameRate(const Json::Value& parsed_json, bool streamVid)
{
    if(streamVid)
    {
        return parsed_json[0]["FindVideo"]["entities"][0][VDMS_VIDFRMRATE].asDouble();
    }
    else
    {
        return parsed_json[0]["FindEntity"]["entities"][0][VDMS_VIDFRMRATE].asDouble();
    }
}


std::vector<frameInfo_obj> VDMSConverter::post_find_qry(
    double& frameRate,
    int numBB,
    Json::Value& parsed_json,
    int qry_ofst,
    bool streamVid
) {
    frameRate = getVidFrameRate(parsed_json, streamVid);
    std::vector<frameInfo_obj> frameInfoArr;
    frameInfo_obj frameInfo;
    frameInfo.frameID = parsed_json[qry_ofst]["FindBoundingBox"]["entities"][0][VDMS_FRAMEID].asInt();
    for(int i = 0; i < numBB; i++)
    {
        int curFrameID = parsed_json[qry_ofst]["FindBoundingBox"]["entities"][i][VDMS_FRAMEID].asInt();
        bbox_obj bbox;
        bbox.x = parsed_json[qry_ofst]["FindBoundingBox"]["entities"][i]["_coordinates"]["x"].asInt();
        bbox.y = parsed_json[qry_ofst]["FindBoundingBox"]["entities"][i]["_coordinates"]["y"].asInt();
        bbox.w = parsed_json[qry_ofst]["FindBoundingBox"]["entities"][i]["_coordinates"]["w"].asInt();
        bbox.h = parsed_json[qry_ofst]["FindBoundingBox"]["entities"][i]["_coordinates"]["h"].asInt();
        if(curFrameID == frameInfo.frameID)
        {
            frameInfo.bbox_obj_arr.push_back(bbox);
        }
        else
        {
            frameInfoArr.push_back(frameInfo);
            frameInfo.bbox_obj_arr.clear();
            frameInfo.frameID = curFrameID;
            frameInfo.bbox_obj_arr.push_back(bbox);
        }
    }
    frameInfoArr.push_back(frameInfo);
    std::cout << "Found " << numBB << " bounding boxes" << std::endl;
    return frameInfoArr;
}


std::vector<frameInfo_obj> VDMSConverter::post_find_ppl_qry(
    double& frameRate,
    int n,
    bool numPPL_exact,
    int numFrames,
    Json::Value& parsed_json,
    int qry_ofst,
    bool streamVid
) {
    std::vector<frameInfo_obj> frameInfoArr;
    frameRate = getVidFrameRate(parsed_json, streamVid);
    for(int i = 0; i < numFrames; i++)
    {
        int numBB = parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["returned"].asInt();
        if(numPPL_exact)
        {
            frameInfo_obj frameInfo;
            std::vector<bbox_obj> &bbox_obj_arr = frameInfo.bbox_obj_arr;
            for(int j = 0; j < numBB; j++)
            {
                if(parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j][VDMS_OBJID] == VDMS_OBJID_PERSON)
                {
                    bbox_obj bbox;
                    bbox.x = parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j]["_coordinates"]["x"].asInt();
                    bbox.y = parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j]["_coordinates"]["y"].asInt();
                    bbox.w = parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j]["_coordinates"]["w"].asInt();
                    bbox.h = parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j]["_coordinates"]["h"].asInt();
                    bbox_obj_arr.push_back(bbox);
                }
            }
            if(bbox_obj_arr.size() == n)
            {
                frameInfo.frameID = parsed_json[(i * 2) + qry_ofst]["FindEntity"]["entities"][0][VDMS_FRAMEID].asInt();
                frameInfoArr.push_back(frameInfo);
            }
        }
        else if(!numPPL_exact)
        {
            frameInfo_obj frameInfo;
            std::vector<bbox_obj> &bbox_obj_arr = frameInfo.bbox_obj_arr;
            for(int j = 0; j < numBB; j++)
            {
                if(parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j][VDMS_OBJID] == VDMS_OBJID_PERSON)
                {
                    bbox_obj bbox;
                    bbox.x = parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j]["_coordinates"]["x"].asInt();
                    bbox.y = parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j]["_coordinates"]["y"].asInt();
                    bbox.w = parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j]["_coordinates"]["w"].asInt();
                    bbox.h = parsed_json[(i * 2 + 1) + qry_ofst]["FindBoundingBox"]["entities"][j]["_coordinates"]["h"].asInt();
                    bbox_obj_arr.push_back(bbox);
                }
            }
            if(bbox_obj_arr.size() != 0)
            {
                frameInfo.frameID = parsed_json[(i * 2) + qry_ofst]["FindEntity"]["entities"][0][VDMS_FRAMEID].asInt();
                frameInfoArr.push_back(frameInfo);
            }
        }
    }
    std::cout << "Found " << frameInfoArr.size() << " frames" << std::endl;
    return frameInfoArr;
}


int VDMSConverter::post_upld_vid_qry(int sendQry_retV, const std::string& video_id, bool streamVid)
{
    if(sendQry_retV == 0)
    {
        return 0;
    }
    else if(streamVid)
    {
        std::cout << "Uploading " << video_id << " to VDMS server" << std::endl;
        Json::Value qry;
        int qry_ref = 1;
        double frameRate = getVidFrameRate(video_id);
        if(frameRate == -1.0f)
        {
            return -1;
        }
        Json::Value props;
        props[VDMS_VIDFRMRATE] = frameRate;
        props[VDMS_VIDNAME] = video_id;
        add_video(qry, qry_ref, video_id, props);
        std::string* serialVid = serialVideo(video_id);
        std::vector<std::string*> blobs;
        blobs.push_back(serialVid);
        Json::Value parsed_json;
        VDMS::Response resp;
        return send_qry(parsed_json, qry, resp, blobs);
    }
    else
    {
        return sendQry_retV;
    }
}


int VDMSConverter::send_qry(
    Json::Value& parsed_json,
    const Json::Value& qry,
    VDMS::Response& resp,
    const std::vector<std::string*>& blobs
) {
    if(_dumpTransX)
    {
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "Query" << _numTransX << std::endl;
        std::cout << qry << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
    }
    Json::FastWriter fastwriter;
    resp = _aclient->query(fastwriter.write(qry), blobs);
    Json::Reader reader;
    reader.parse(resp.json.c_str(), parsed_json);
    if(_dumpTransX)
    {
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "Query Response" << _numTransX << std::endl;
        std::cout << parsed_json << std::endl;
	std::cout << std::endl;
        std::cout << std::endl;
    }
    if(!parsed_json.isArray())
    {
        if(_dumpTransX)
        {
            _numTransX++;
        }
        return -1;
    }
    for(int i = 0; i < parsed_json.size(); i++)
    {
        Json::Value::Members qryNameArr = parsed_json[i].getMemberNames();

        for(int j = 0; j < qryNameArr.size(); j++)
        {
            std::string qryName = qryNameArr[j];
            if(qryName == "FailedCommand" || (parsed_json[i][qryName]["status"].asInt() == -1))
            {
                if(_dumpTransX)
                {
                    _numTransX++;
                }
                return -1;
            }
        }
    }
    if(_dumpTransX)
    {
        _numTransX++;
    }
    return 0;
}


std::string* VDMSConverter::serialVideo(const std::string& video_path)
{
    std::ifstream fd(video_path);
    std::string *fileContents = new std::string(
        (std::istreambuf_iterator<char>(fd)),
        std::istreambuf_iterator<char>()
    );
    fd.close();
    return fileContents;
}


std::string VDMSConverter::setFileName(const std::string& video_path)
{
    std::string output_video_fn = video_path;
    int dotIdx = output_video_fn.find_last_of(".");
    output_video_fn = output_video_fn.substr(0, dotIdx);
    int slashIdx = output_video_fn.find_last_of("/");
    int length = (output_video_fn.length() - slashIdx);
    output_video_fn = output_video_fn.substr((slashIdx + 1), length);
    output_video_fn = "sum_" + output_video_fn + ".mp4";
    return output_video_fn;
}


int VDMSConverter::write_video(
    std::vector<frameInfo_obj>& frameInfoArr,
    bool showBB,
    const std::string& video_id,
    bool streamVid
) {
    int frame_height;
    int frame_width;
    cv::VideoCapture *cap = NULL;
    double frameRate = getVidFrameRate(video_id);
    if(streamVid)
    {
        if(downloadFrames(frameInfoArr, video_id))
        {
            std::cout << "Could not download Frames" << std::endl;
            return -1;
        }
        frame_height = frameInfoArr[0].frame.rows;
        frame_width  = frameInfoArr[0].frame.cols;
    }
    else
    {
        cap = new cv::VideoCapture(video_id);
        if(!cap->isOpened())
        {
            std::cout << "Could not find " << video_id << std::endl;
            delete cap;
            return -1;
        }
        frame_height = (int) cap->get(cv::CAP_PROP_FRAME_HEIGHT);
        frame_width  = (int) cap->get(cv::CAP_PROP_FRAME_WIDTH);
    }
    std::string output_video_fn = setFileName(video_id);
    std::cout << "Creating Video: " << output_video_fn << std::endl;
    cv::VideoWriter outVideo(
        output_video_fn,
        cv::VideoWriter::fourcc('a','v','c','1'), //'H','2','6','4'),
        frameRate,
        cv::Size(frame_width, frame_height)
    );
    for(int i = 0; i < frameInfoArr.size(); i++)
    {
        frameInfo_obj& frameInfo = frameInfoArr[i];
        cv::Mat frame;
        if(streamVid)
        {
            frame = frameInfo.frame;
        }
        else
        {
            cap->set(cv::CAP_PROP_POS_FRAMES, frameInfo.frameID);
            cap->read(frame);
        }
        if(showBB)
        {
            for(int j = 0; j < frameInfo.bbox_obj_arr.size(); j++)
            {
                bbox_obj bbox = frameInfo.bbox_obj_arr[j];
                cv::Point top_left_pt(bbox.x, bbox.y);
                cv::Point bottom_right_pt(bbox.x + bbox.w, bbox.y + bbox.h);
                cv::rectangle(frame, top_left_pt, bottom_right_pt, cv::Scalar(0, 255, 0), 3);
            }
        }
        outVideo.write(frame);
    }
    outVideo.release();
    delete cap;
    return 0;
}