/* Simple WiFi Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <cstring>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <esp_wifi_types.h>
#include "sdkconfig.h"

#include "apa102.h"


/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_MODE_AP   CONFIG_ESP_WIFI_MODE_AP //TRUE:AP FALSE:STA
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN       CONFIG_MAX_STA_CONN

#define MESSAGE "Hello TCP Client!!"
#define LISTENQ 2
const int CONNECTED_BIT = BIT0;

//unsigned char g_light_command = 255;
uint8_t g_light_command = 1;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "ringlight";


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "got ip:%s",
                     ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                     MAC2STR(event->event_info.sta_connected.mac),
                     event->event_info.sta_connected.aid);
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
                     MAC2STR(event->event_info.sta_disconnected.mac),
                     event->event_info.sta_disconnected.aid);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}


void wifi_init_softap()
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config));
    wifi_config.ap.ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID);
    memcpy(&wifi_config.ap.ssid, EXAMPLE_ESP_WIFI_SSID, strlen(EXAMPLE_ESP_WIFI_SSID) );
    memcpy(&wifi_config.ap.password, EXAMPLE_ESP_WIFI_PASS, strlen(EXAMPLE_ESP_WIFI_PASS));
    wifi_config.ap.max_connection = EXAMPLE_MAX_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void wifi_init_sta()
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config));
    memcpy(&wifi_config.sta.ssid, EXAMPLE_ESP_WIFI_SSID, strlen(EXAMPLE_ESP_WIFI_SSID) );
    memcpy(&wifi_config.sta.password, EXAMPLE_ESP_WIFI_PASS, strlen(EXAMPLE_ESP_WIFI_PASS));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void tcp_server(void *pvParam){
    ESP_LOGI(TAG,"tcp_server task started \n");
    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons( 3000 );
    int s, r;
    char recv_buf[64];
    static struct sockaddr_in remote_addr;
    static unsigned int socklen;
    socklen = sizeof(remote_addr);
    int cs;//client socket
    xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,false,true,portMAX_DELAY);
    while(1){
        s = socket(AF_INET, SOCK_STREAM, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.\n");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket\n");
        if(bind(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0) {
            ESP_LOGE(TAG, "... socket bind failed errno=%d \n", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket bind done \n");
        if(listen (s, LISTENQ) != 0) {
            ESP_LOGE(TAG, "... socket listen failed errno=%d \n", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        int reuse = 1;
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        //perror("setsockopt(SO_REUSEADDR) failed");
  //https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr/25193462
        ESP_LOGI(TAG,"setsockopt(SO_REUSEADDR) failed");

        std::string conn_msg("IAI ringlight controller.\n");

        while(1){
            cs = accept(s, (struct sockaddr *)&remote_addr, &socklen);
            ESP_LOGI(TAG, "New connection request, Request data:");
            //set O_NONBLOCK so that recv will return, otherwise we need to implement message end
            //detection logic. If know the client message format you should instead implement logic
            //detect the end of message
            bool first_byte_recorded = false;
            char second_byte, first_byte = 'x';


            write(cs, conn_msg.c_str(), conn_msg.length()) ;



            //mnk-------------
            char *answer[50];
            bzero(recv_buf, sizeof(recv_buf));
            while( (r = recv(cs , recv_buf , sizeof(recv_buf) , 0)) > 0 )
            {

                first_byte = recv_buf[0];
                second_byte = recv_buf[1];
                if (second_byte == 'x'){
                    write(cs , "goodbye, connection ending <\n " , 27);
                    close(cs);
                    break;
                }
                ESP_LOGI(TAG,"received=%d ,  r= %d", (first_byte -'0'),r);
                if (first_byte >= '0' && first_byte <= '9')  //  '0' = 48   '9' = 58
                {
                    g_light_command = (first_byte -'0')*28;
                    if (first_byte == '9') {
                        g_light_command=255;
                    }
                
                
                
                ESP_LOGI(TAG,"In loop Setting light to: %i\n", g_light_command);
                bzero(answer, sizeof(answer));
                snprintf((char*)answer, sizeof(answer) - 1, "Setting light to: %i\n", g_light_command);

                if( write(cs , answer , strlen((const char*)answer)) < 0)
                {
                    ESP_LOGE(TAG, "... Send failed \n");
                    close(cs);
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    continue;
                }                
                
                
                }
                else{
                    //g_light_command =255;
                    ESP_LOGI(TAG,"out of range input");
                    write(cs , "out of range, not changing\n<" , 28);
                }







                //Send the message back to client
                ///write(cs , recv_buf , strlen(recv_buf));
            }
            // mnk------------


            ESP_LOGI(TAG, "... socket send success");
            close(cs);
        }
        ESP_LOGI(TAG, "... server will be opened in 5 seconds");
        vTaskDelay(60 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "...tcp_client task closed\n");
}


void white_w_brightness(apa102 &oAPA, uint8_t blevel) {
    static unsigned int report_counter=0;
    report_counter += 1;

    if (report_counter > 5) {
        ESP_LOGI(TAG, "setting all white, with brightness %d. XXX", blevel);
        report_counter = 0;
    }

    apa102::colorRGBB color;

    color.red=blevel;
    color.green=blevel;
    color.blue=blevel;
    color.brightness=31; //31 is the max brightness for apa102 in the slow PWM

    oAPA.setColor(color);

}


static void apa102_task(void *pvParameters) {
    apa102 oAPA102(8);  //Number of LEDs in the chain
   // oAPA102.ramdomBlink(500,300); // works -- random one blinks
//    oAPA102.ramdomBackWalk(500,300), //single led lit, position rotates .. 1 missing
//      oAPA102.ramdomWalk(500,300);  //single led rotate, all led works
//    oAPA102.ramdomFade(500,300);
//    oAPA102.test();
//    oAPA102.test2();
//    oAPA102.ramdomStepR(500,300);
//    oAPA102.ramdomStep_mnk(500,300,3);   //third param num has to be 0-7
    oAPA102.ramdomWalk_mnk(500,300,4,g_light_command);




//    while(1) {
//        //oAPA102.white_w_brightness(g_light_command);
//        white_w_brightness(oAPA102, g_light_command);
//        vTaskDelay(250/portTICK_PERIOD_MS);
//    }
    //esp_task_wdt_delete();
    vTaskDelete(NULL);
}

extern "C" {
void app_main();
}


void app_main() {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#if EXAMPLE_ESP_WIFI_MODE_AP
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
#else
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
#endif /*EXAMPLE_ESP_WIFI_MODE_AP*/


    xTaskCreate(&tcp_server, "tcp_server", 4096, NULL, 5, NULL);
    xTaskCreate(&apa102_task, "mqtt_task", 12288, NULL, 5, NULL);
}
