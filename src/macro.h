#define IM_W (71)
#define IM_H (71)
#define IM_LEN (IM_W * IM_H)
#define AMG8833_TEMP_FACTOR (256)
#define DHT11_PIN GPIO_NUM_25 

//#define UART_SIM
#define ENABLE_NETWORK

#define DEBUG 0

#if DEBUG
#define DBG_PRINT(format, args...)\                             
        do{\
        printf(format, ##args);\
        }while (0)
#else
#define DBG_PRINT(format, args...)
#endif

///* home
#define MYSSID "Racoon&Masters"
#define MYPWD "gotomunich2019"
#define MYMQTT "mqtt://192.168.178.42:1883"
//*/
/*Professor's room
#define MYSSID "CAPS"
#define MYPWD "caps!schulz-wifi"
#define HIVEMQ_USERNAME "ircamera"
#define HIVEMQ_PASSWORD "tumCAPS2021"
#define MYMQTT "mqtt://broker.hivemq.com:1883" //"mqtt://131.159.85.131:1883"
//*/
/* my phone
#define MYSSID "JOJO!"
#define MYPWD "11111111"
#define MYMQTT "mqtt://192.168.43.197:1883"
//*/
/* seminar room
#define MYSSID "capslab"
#define MYPWD "12356789"
#define MYMQTT "mqtt://172.20.10.4:1883"
//*/

#define IOT_IP "131.159.35.132"
#define IOT_PORT "1883"
#define IOT_USERNAME "JWT"
#define IOT_PASSWORD "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2MjgxNTU3NTUsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiNTZfMTQyIn0.On7oZ3P2qF2vCAdQlcFDxf_5pY6713vJSl5rVEwq1qcukhdyQeHJEugIYzTt3UPkw7kUgjmpKw5-QLI3wYPJq1rRdCNkb3dChl-MaFSwVtJKMXCUjvrS5CVs9APdfDrndE96sZB4oLaKYyLmYP9gvROLiQ62KN_gNtyHZOGUy-_SfXI6koEGcP1rnu4o_9KRfYP8QcjmHJQfB74Q4XTvS_yqnW_AuHM6eigE5xWvMCS3VvKAz3nVboiJBiM0LyeJL1mXh6yaFACImPramu3RBQIzjyDHgcf3-WMhXPRQJF1XX5XcmhCoaJuwDyIMpL7aiEajx_Z1qZtCjUmwUrkMuKWXNuNFnSN08S4BF0ky6qKriN4V0dcne4kEeojSm3dHE98NFcGtVrycyuWSvds6K4x7QyLLZk-jx1qvXT-VzFfpU7L9oqbf_iN6Xd_VA9rkNpWYDbKUhqbvqM6I5-RONfF-NiM_qGt5d-lJJTmYlbEYVxYFamnsX1sXJBemIPJNsoBUQQ7sSaSSYGxEqd_ojnimWUMQCfombsvjYx6SQDod7bEn3UqPUxMK3DU4bPbQu5sSE3utAHTZEO-qSxYgydcbbI0r9KwP53URjYK-o155jzTbpqo_3sSTqCe49WX4GwgNZAetKvd1ze-B7QfVBONWdTjbTWPA0ExqFBt1n_c"
#define IOT_DEVICE_ID "142"
#define IOT_USER_ID "56"
#define IOT_USERNAME_PUB "chenyuan"
#define IOT_TOPIC IOT_USER_ID "_" IOT_DEVICE_ID
#define IOT_URI "mqtt://" IOT_USERNAME ":" IOT_PASSWORD "@" IOT_IP ":" IOT_PORT
#define IOT_MESSAGE_PATTERN "{\"username\":\"%s\",\"%s\":%d,\"device_id\":%s,\"timestamp\":%lld}"
#define IOT_SENSOR "count"