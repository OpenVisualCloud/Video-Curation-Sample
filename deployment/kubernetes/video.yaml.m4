
apiVersion: v1
kind: Service
metadata:
  name: video-service
  labels:
    app: video
spec:
  ports:
    - port: 8080
      targetPort: 8080
      name: http
  selector:
    app: video

---

apiVersion: apps/v1
kind: Deployment
metadata:
  name: video
  labels:
     app: video
spec:
  replicas: 1
  selector:
    matchLabels:
      app: video
  template:
    metadata:
      labels:
        app: video
    spec:
      enableServiceLinks: false
      containers:
        - name: video
          image: defn(`REGISTRY_PREFIX')lcc_video:latest
          imagePullPolicy: IfNotPresent
          ports:
            - containerPort: 8080
          env:
            - name: RETENTION_MINS
              value: "60"
            - name: CLEANUP_INTERVAL
              value: "10m"
            - name: KKHOST
              value: "kafka-service:9092"
            - name: `INGESTION'
              value: "defn(`INGESTION')"
          volumeMounts:
            - mountPath: /etc/localtime
              name: timezone
              readOnly: true
      volumes:
        - name: timezone
          hostPath:
            path: /etc/localtime
            type: File
