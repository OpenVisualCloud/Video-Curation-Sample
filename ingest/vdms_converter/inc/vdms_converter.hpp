#pragma once


// Definitions
#define UNUSED(x) (void)(x)
#define VDMS_VIDNAME                "name"
#define VDMS_VIDFRMRATE             "frameRate"
#define VDMS_FRAMEID                "frameID"
#define VDMS_OBJID                  "objectID"
#define VDMS_OBJID_PERSON           "person"
#define VDMS_OBJID_FACE             "face"
#define VDMS_AGE                    "age"
#define VDMS_GEND                   "gender"
#define VDMS_GEND_MALE              "male"
#define VDMS_GEND_FEMALE            "female"
#define VDMS_EMOT                   "emotion"
#define VDMS_EMOT_NEUTRAL           "neutral"
#define VDMS_EMOT_HAPPY             "happy"
#define VDMS_EMOT_SAD               "sad"
#define VDMS_EMOT_SURPRISE          "surprise"
#define VDMS_EMOT_ANGER             "anger"
#define VDMS_CLASS_FRAME            "Frame"
#define VDMS_CLASS_VIDEO            "Video"
#define VDMS_EDGE_CLASS_VID2FRAME   "Vid2Frame"
#define VDMS_EDGE_CLASS_FRME2BB     "Frame2BB"
#define VDMS_TX_FRAME_BATCH_SIZE    25


// System Includes
#include <fstream>
#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <regex>
#include <iomanip>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/videoio.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/highgui/highgui.hpp>


// Project Includes
#include "VDMSClient.h"


class bbox_obj
{
    public:
        int x;
        int y;
        int h;
        int w;
        Json::Value props;
};


class frameInfo_obj
{
    public:
        int frameID;
        cv::Mat frame;
        std::vector<bbox_obj> bbox_obj_arr;
};


class VDMSConverter
{
    public:
        VDMSConverter(std::string& server_address, bool dumpTransX = false);


        ~VDMSConverter();


        static void add_bbox_props(int age, bbox_obj& bbox, const std::string& emotion, int frameID, const std::string& gender);


        static std::vector<std::string> crtObjIDArr(char* fname, std::vector<std::string> coco_labels);


        static double getVidFrameRate(std::string video_id);


        std::vector<frameInfo_obj> find_emotion(const std::string& emotion, bool showBB, const std::string& video_id, bool streamVid = true);


        std::vector<frameInfo_obj> find_objects(std::vector<std::string> objIDArr, bool showBB, const std::string& video_id, bool streamVid = true);


        std::vector<frameInfo_obj> find_people(int n, bool numPPL_exact, bool showBB, const std::string& video_id, bool streamVid = true);


        int upload_data(const bbox_obj& bbox, int frameID, const std::string& video_id, bool streamVid = true);


        int upload_video(const std::string& video_id, bool streamVid = true);

        int write_video(
            std::vector<frameInfo_obj>& frameInfoArr,
            bool showBB,
            const std::string& video_id,
            bool streamVid
        );

    private:
        int add_bbox(const bbox_obj& bbox, Json::Value& qry, int qry_ref);


        int add_connection(const std::string& edgeClass, int node_ref1, int node_ref2, Json::Value& qry, const Json::Value& props);


        void add_constraints(Json::Value& json_node, const Json::Value& constraints = Json::Value());


        int add_frame_entity(int frameID, Json::Value& qry, int qry_ref, const std::string& video_id);


        void add_props(Json::Value& json_node, const Json::Value& props = Json::Value());


        int add_video(Json::Value& qry, int qry_ref, const std::string& video_id, const Json::Value& props = Json::Value());


        int add_video_entity(Json::Value& qry, int qry_ref, const std::string& video_id, const Json::Value& props = Json::Value());


        int crte_add_bbox_qry(const bbox_obj& bbox, int frameID, bool newFrame, Json::Value& qry, const std::string& video_id, bool streamVid);


        int crte_find_emot_frmes_qry(std::string emotion, Json::Value& qry, const std::string& video_id, bool streamVid = true);


        int crte_find_obj_frmes_qry(std::vector<std::string> objIDArr, Json::Value& qry, const std::string& video_id, bool streamVid = true);


        void crte_find_frame_ent_qry(int frameID, Json::Value& qry, const std::string& video_id, bool streamVid);


        int crte_find_vid_frmes_ent_qry(Json::Value& qry, const std::string& video_id, bool streamVid = true);


        int crte_find_ppl_frmes_qry(
            int numFrames,
            Json::Value parsed_json,
            Json::Value& qry,
            int qry_ofst,
            const std::string& video_id,
            bool streamVid = true
        );


        int crte_upld_data_qry(const bbox_obj& bbox, int frameID, bool newFrame, Json::Value& qry, const std::string& video_id, bool streamVid = true);


        int crte_upld_vid_qry(Json::Value& qry, const std::string& video_path, bool streamVid = true);


        void deserialFrames(std::vector<frameInfo_obj>& frameInfoArr, int ofst, const VDMS::Response& resp);


        int downloadFrames(std::vector<frameInfo_obj>& frameInfoArr, const std::string& video_id);


        void find_bbox(
            int link_ref,
            Json::Value& qry,
            int& qry_ref,
            const Json::Value& constraints = Json::Value(),
            const Json::Value& props = Json::Value(),
            const Json::Value& res = Json::Value()
        );


        void findConnectedEntity(
            std::string edge_class,
            std::string entity_class,
            int link_ref,
            Json::Value& qry,
            int qry_ref,
            const Json::Value& props = Json::Value(),
            const Json::Value& res = Json::Value()
        );


        int find_frames(const std::vector<int>& frameIDArr, Json::Value& qry, int qry_ref, const std::string& video_id);

        int find_frame_entity(
            int link_ref,
            Json::Value& qry,
            int qry_ref,
            const Json::Value& constraints = Json::Value(),
            const Json::Value& props = Json::Value(),
            const Json::Value& res = Json::Value()
        );

        int find_video(
            Json::Value& qry,
            int qry_ref,
            const Json::Value& constraints = Json::Value(),
            bool streamVid = true,
            const Json::Value& props = Json::Value()
        );

        int find_video(
            Json::Value& qry,
            int qry_ref,
            const Json::Value& constraints = Json::Value(),
            const Json::Value& props = Json::Value()
        );

        int find_video_entity(
            Json::Value& qry,
            int qry_ref,
            const Json::Value& constraints = Json::Value(),
            const Json::Value& props = Json::Value()
        );

        std::string getExtenstion(const std::string& video_id);

        void get_props(Json::Value& json_node, Json::Value props = Json::Value());

        void set_results(Json::Value& json_node, const Json::Value& res);

        double getVidFrameRate(const Json::Value& parsed_json, bool streamVid = true);

        std::vector<frameInfo_obj> post_find_qry(
            double& frameRate,
            int numBB,
            Json::Value& parsed_json,
            int qry_ofst,
            bool streamVid = true
        );

        std::vector<frameInfo_obj> post_find_ppl_qry(
            double& frameRate,
            int n,
            bool numPPL_exact,
            int numFrames,
            Json::Value& parsed_json,
            int qry_ofst,
            bool streamVid = true
        );

        int post_upld_vid_qry(int sendQry_retV, const std::string& video_id, bool streamVid = true);

        int send_qry(
            Json::Value& parsed_json,
            const Json::Value& qry,
            VDMS::Response& resp,
            const std::vector<std::string*>& blobs = std::vector<std::string*>()
        );

        std::string* serialVideo(const std::string& video_path);

        std::string setFileName(const std::string& video_path);

        std::shared_ptr<VDMS::VDMSClient> _aclient;
        int _numTransX;
        bool _dumpTransX;
};

