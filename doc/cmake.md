
### CMake Options:

Use the following definitions to customize the building process:   
- **PLATFORM**: Specify the target platform: `Xeon`   
- **NCURATIONS**: Specify the number of curation processes running in the background.  
- **INGESTION**: Specify the ingestion mode: `face` and/or `object`. Use comma as the deliminator to specify more than 1 ingestion mode.   

### Examples:   

```
cd build
cmake -DPLATFORM=Xeon ..
```

### Make Commands:

- **build**: Build the sample (docker) images.  
- **update**: Distribute the sample images to worker nodes.  
- **dist**: Create the sample distribution package.   
- **start/stop_docker_compose**: Start/stop the sample orchestrated by docker-compose.  
- **start/stop_docker_swarm**: Start/stop the sample orchestrated by docker swarm.   
- **start/stop_kubernetes**: Start/stop the sample orchestrated by Kubernetes.   

### See Also:

- [Sample Distribution](dist.md)   

