
    video-service:
        image: defn(`REGISTRY_PREFIX')lcc_video:latest
        environment:
            RETENTION_MINS: "60"
            CLEANUP_INTERVAL: "10m"
            KKHOST: "kafka-service:9092"
            `INGESTION': "defn(`INGESTION')"
        volumes:
            - /etc/localtime:/etc/localtime:ro
        networks:
            - appnet
        restart: always
