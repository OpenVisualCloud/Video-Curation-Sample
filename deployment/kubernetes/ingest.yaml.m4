
apiVersion: apps/v1
kind: Deployment
metadata:
  name: ingest
  labels:
     app: ingest
spec:
  replicas: defn(`NCURATIONS')
  selector:
    matchLabels:
      app: ingest
  template:
    metadata:
      labels:
        app: ingest
    spec:
      enableServiceLinks: false
      containers:
        - name: ingest
          image: defn(`REGISTRY_PREFIX')lcc_ingest:latest
          imagePullPolicy: IfNotPresent
          env:
            - name: KKHOST
              value: "kafka-service:9092"
            - name: VDHOST
              value: "http://video-service:8080"
            - name: DBHOST
              value: "vdms-service"
            - name: ZKHOST
              value: "zookeeper-service:2181"
            - name: NO_PROXY
              value: "video-service,${NO_PROXY}"
            - name: no_proxy
              value: "video-service,${no_proxy}"
          volumeMounts:
            - mountPath: /etc/localtime
              name: timezone
              readOnly: true
      volumes:
        - name: timezone
          hostPath:
            path: /etc/localtime
            type: File
