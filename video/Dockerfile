
FROM openvisualcloud/xeon-ubuntu1804-media-nginx:20.7

RUN apt-get update && apt-get install -y -q python3-tornado python3-requests python3-ply python3-psutil kafkacat curl && rm -rf /var/lib/apt/lists/*

COPY *.py /home/
COPY cleanup.sh /home/
COPY manage.sh /home/
COPY *.conf /etc/nginx/
COPY archive/*.mp4 /var/www/mp4/
CMD  ["/home/manage.sh"]

####
ARG  USER
ARG  GROUP
ARG  UID
ARG  GID
## must use ; here to ignore user exist status code
RUN  [ ${GID} -gt 0 ] && groupadd -f -g ${GID} ${GROUP}; \
     [ ${UID} -gt 0 ] && useradd -d /home/${USER} -g ${GID} -K UID_MAX=${UID} -K UID_MIN=${UID} ${USER}; \
     touch /var/run/nginx.pid && \
     mkdir -p /var/log/nginx /var/lib/nginx /var/www/cache /var/www/gen /var/www/mp4 && \
     chown -R ${UID}.${GID} /var/run/nginx.pid /var/log/nginx /var/lib/nginx /var/www /etc/nginx/nginx.conf
VOLUME ["/var/www"]
USER ${UID}
####
