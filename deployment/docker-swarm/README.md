
The sample can be deployed with either docker-compose (v1.20+ required) or docker swarm. The deployment uses the same configuration script.   

### Docker-Compose Deployment

This is as simple as 

```
make start_docker_compose
make stop_docker_compose
```

### Docker Swam Deployment

Follow the [instructions](https://docs.docker.com/engine/swarm/swarm-tutorial/create-swarm) to create a swarm. Then setup the swarm as follows:     

- Optionally, only required by `make update`, setup password-less acess from the swarm manager to each swarm node:   

```
ssh-keygen
ssh-copy-id <worker>
```

Start/stop services as follows:   

```
make update # optional for private registry
make start_docker_swarm
make stop_docker_swarm
```

---

The command `make update` uploads the sample images to each worker node. If you prefer to use a private docker registry, configure the sample, `cmake -DREGISTRY=<registry-url> ..`, to push the sample images to the private docker registry after each build.

---

### See Also 

- [Utility Scripts](../../doc/script.md)   
