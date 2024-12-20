
apiVersion: v1
kind: Service
metadata:
  name: zookeeper-service
  labels:
    app: zookeeper
spec:
  ports:
  - port: 2181
    protocol: TCP
  selector:
    app: zookeeper

---

apiVersion: apps/v1
kind: Deployment
metadata:
  name: zookeeper
  labels:
     app: zookeeper
spec:
  replicas: 1
  selector:
    matchLabels:
      app: zookeeper
  template:
    metadata:
      labels:
        app: zookeeper
    spec:
      enableServiceLinks: false
      containers:
        - name: zookeeper
          image: zookeeper:3.5.6
          imagePullPolicy: IfNotPresent
          ports:
            - containerPort: 2181
          env:
            - name: "ZOO_TICK_TIME"
              value: "10000"
            - name: "ZOO_MAX_CLIENT_CNXNS"
              value: "160000"
            - name: "ZOO_LOG4J_PROP"
              value: "ERROR"

