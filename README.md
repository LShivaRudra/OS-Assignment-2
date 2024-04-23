# OS-Assignment-2
## AIR TRAFFIC CONTROL SYSTEM

---

<img alt="image" width="821" src="flowchart.png">


## Instructions for Running:
1. Create 6 terminals:
    1. 1 cleanup
    2. 1 ATC
    3. 2 airports
    4. 2 planes

    however, you can create any number of planes and airports you want.

2. Type the following commands in the terminal for the respective `.c` file:
    1. **cleanup.c** :
    ``` 
        gcc cleanup.c -o cleanup
        ./cleanup
    ```
    2. **airtrafficcontroller.c** :
    ``` 
        gcc airtrafficcontroller.c -o atc
        ./atc
    ```
    3. **airport.c** :
    ```
        gcc airport.c -o airport -lpthread
        ./airport
    ```
    4. **plane.c** : 
    ``` 
        gcc plane.c -o plane
        ./plane
    ```
## The `msg-type` can be assigned as follows:
1. Planes : 1-10
2. ATC: 22
3. Airport: 10+`aipord_id`
4. Cleanup: 21