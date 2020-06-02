
    ingest:
        image: defn(`REGISTRY_PREFIX')lcc_ingest:latest
        environment:
            KKHOST: "kafka-service:9092"
            VDHOST: "http://video-service:8080"
            DBHOST: "vdms-service"
            ZKHOST: "zookeeper-service:2181"
            no_proxy: "video-service,${no_proxy}"
            NO_PROXY: "video-service,${NO_PROXY}"
        volumes:
            - /etc/localtime:/etc/localtime:ro
        networks:
            - appnet
        restart: always
        deploy:
            replicas: defn(`NCURATIONS')
