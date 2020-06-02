
FROM openvisualcloud/xeon-ubuntu1804-analytics-dev:20.4 as build

RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y -q libgoogle-glog-dev libgflags-dev libgtest-dev libjsoncpp-dev cmake git scons libprotobuf-dev protobuf-compiler

# build gtest
RUN cd /usr/src/gtest && cmake . && make -j$(nproc) && mv libgtest* /usr/lib

# vdms client
ENV LD_LIBRARY_PATH /opt/vdms/utils:/opt/vdms/client/cpp:${LD_LIBRARY_PATH}
RUN cd /opt && git clone https://github.com/IntelLabs/vdms && cd vdms && git checkout tags/v2.0.1 && mkdir db && scons -j$(nproc) INTEL_PATH=/opt --no-server

# vdms converter
COPY vdms_converter /opt/vdms_converter
RUN cd /opt/vdms_converter && scons -j$(nproc)

# metaData_extract
COPY gstreamer_gva /opt/gstreamer_gva
RUN cd /opt/gstreamer_gva && scons --metaData -Q VDMS_ROOT_PATH=/opt/vdms/ GST_GVA_PLUGIN_ROOT_PATH=/usr/local/lib/x86_64-linux-gnu/gstreamer-1.0

# final image
FROM openvisualcloud/xeon-ubuntu1804-analytics-gst:20.4

# prerequisite
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y -q libjsoncpp1 python3-pip wget libdc1394-22 libprotobuf10 && rm -rf /var/lib/apt/lists/* && pip3 install 'kafka-python>=1.4.7' 'kazoo>=2.6.1'

COPY --from=build /opt/vdms_converter/libvdms_converter.so /usr/local/lib/libvdms_converter.so
COPY --from=build /opt/vdms/client/cpp/libvdms-client.so /usr/local/lib/libvdms-client.so
COPY --from=build /opt/vdms/utils/libvdms-utils.so /usr/local/lib/libvdms-utils.so
COPY --from=build /opt/gstreamer_gva/metaData_extract/build/debug/metaData_extract /opt/gstreamer_gva/metaData_extract

# models
ENV MODELS_PATH /opt/openvino_models/2019R3_models
COPY openvino_models /opt/openvino_models

# ingest script
COPY *.py /home/
CMD ["/home/ingest.py"]

