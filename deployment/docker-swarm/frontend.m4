
    frontend-service:
        image: defn(`REGISTRY_PREFIX')lcc_frontend:latest
        ports:
            - target: 8443
              published: 443
              protocol: tcp
              mode: host
        environment:
            DBHOST: "vdms-service"
            VDHOST: "http://video-service:8080"
            no_proxy: "video-service,${no_proxy}"
            NO_PROXY: "video-service,${NO_PROXY}"
        volumes:
            - /etc/localtime:/etc/localtime:ro
        secrets:
            - source: self_crt
              target: self.crt
              uid: ${USER_ID}
              gid: ${GROUP_ID}
              mode: 0444
            - source: self_key
              target: self.key
              uid: ${USER_ID}
              gid: ${GROUP_ID}
              mode: 0440
        networks:
            - appnet
        restart: always
        deploy:
            placement:
                constraints:
                    - node.role==manager
