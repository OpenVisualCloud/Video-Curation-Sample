
    vdms-service:
        image: intellabs/vdms:base
        command: ["/bin/sh","-c","cd /vdms;vdms"]
        volumes:
            - /etc/localtime:/etc/localtime:ro
        networks:
            - appnet
        restart: always

