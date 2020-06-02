
The Library Curation sample can be deployed with Kubernetes. 

### Kubernetes Setup

1. Follow the [instructions](https://kubernetes.io/docs/setup) to setup your Kubernetes cluster. If you run into issues with Kubernetes/1.16 and Flannel/master, this [link](https://stackoverflow.com/questions/58024643/kubernetes-master-node-not-ready-state) might help.

2. Optional: Setup password-less access from the Kubernetes controller to each worker node (required by ```make update```):   

```
ssh-keygen
ssh-copy-id <worker-node>
```

3. Finally, start/stop services as follows:   

```
mkdir build
cd build
cmake ..
make
make update # optional for private registry
make start_kubernetes
make stop_kubernetes
```

---

The command `make update` uploads the sample images to each worker node. If you prefer to use a private docker registry, configure the sample, `cmake -DREGISTRY=<registry-url> ..`, to push the sample images to the private docker registry after each build.  

---

### See Also 

- [Utility Scripts](../../doc/script.md)   
