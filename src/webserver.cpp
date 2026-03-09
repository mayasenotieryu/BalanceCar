#include "webserver.h"
#include <Arduino.h>
#include "esp_http_server.h"

extern volatile float web_angle;
extern volatile float web_speed;

static volatile int *joy_x_ptr;
static volatile int *joy_y_ptr;

/* ================= HTML 页面 ================= */

static const char html_page[] =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"

"<style>"

"body{"
"font-family:Arial;"
"background:#1e1e2f;"
"color:white;"
"margin:0;"
"overflow:hidden;"
"}"

"h1{"
"text-align:center;"
"margin:10px;"
"}"

".container{"
"display:flex;"
"flex-direction:column;"
"align-items:center;"
"}"

".row{"
"display:flex;"
"justify-content:center;"
"gap:20px;"
"}"

".card{"
"background:#2c2c3e;"
"padding:15px;"
"border-radius:10px;"
"}"

"#joystick{"
"width:200px;"
"height:200px;"
"background:#444;"
"border-radius:50%;"
"position:relative;"
"touch-action:none;"
"}"

"#stick{"
"width:80px;"
"height:80px;"
"background:#4CAF50;"
"border-radius:50%;"
"position:absolute;"
"left:60px;"
"top:60px;"
"}"

"#graph{"
"background:black;"
"border-radius:10px;"
"}"

".data{"
"font-size:20px;"
"}"

"</style>"
"</head>"

"<body>"

"<h1>ESP32 BalanceCar</h1>"

"<div class='container'>"

"<div class='row'>"

"<div class='card'>"
"<h3>Joystick</h3>"
"<div id='joystick'><div id='stick'></div></div>"
"</div>"

"<div class='card data'>"
"<h3>Data</h3>"
"Angle: <span id='angle'>0</span><br>"
"Speed: <span id='speed'>0</span>"
"</div>"

"</div>"

"<div class='card' style='margin-top:15px;'>"
"<h3>Angle Graph</h3>"
"<canvas id='graph' width='350' height='160'></canvas>"
"</div>"

"</div>"

"<script>"

/* ===== Joystick ===== */

"let joy=document.getElementById('joystick');"
"let stick=document.getElementById('stick');"

"joy.addEventListener('touchmove',function(e){"
"e.preventDefault();"

"let rect=joy.getBoundingClientRect();"

"let x=e.touches[0].clientX-rect.left-100;"
"let y=e.touches[0].clientY-rect.top-100;"

"x=Math.max(-80,Math.min(80,x));"
"y=Math.max(-80,Math.min(80,y));"

"stick.style.left=(x+60)+'px';"
"stick.style.top=(y+60)+'px';"

"fetch('/joy?x='+x+'&y='+y);"
"});"

"joy.addEventListener('touchend',function(){"
"stick.style.left='60px';"
"stick.style.top='60px';"
"fetch('/joy?x=0&y=0');"
"});"

/* ===== Graph ===== */

"let canvas=document.getElementById('graph');"
"let ctx=canvas.getContext('2d');"

"let data=[];"

"function drawGraph(){"

"ctx.fillStyle='black';"
"ctx.fillRect(0,0,350,160);"

"ctx.strokeStyle='lime';"
"ctx.beginPath();"

"for(let i=0;i<data.length;i++){"

"let x=i*3;"
"let y=80-data[i]*40;"   

"if(i==0) ctx.moveTo(x,y);"
"else ctx.lineTo(x,y);"

"}"

"ctx.stroke();"

"}"

/* ===== Data ===== */

"function updateData(){"

"fetch('/data')"
".then(r=>r.json())"
".then(d=>{"

"document.getElementById('angle').innerText=d.angle.toFixed(3);"
"document.getElementById('speed').innerText=d.speed.toFixed(3);"

"data.push(d.angle-1.56);"

"if(data.length>100) data.shift();"

"drawGraph();"

"});"

"}"

"setInterval(updateData,100);"

"</script>"

"</body>"
"</html>";

/* ================= Page ================= */

esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* ================= Joystick interface ================= */

esp_err_t joy_handler(httpd_req_t *req)
{
    char query[64];

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        char xs[16];
        char ys[16];

        httpd_query_key_value(query,"x",xs,sizeof(xs));
        httpd_query_key_value(query,"y",ys,sizeof(ys));

        *joy_x_ptr = atoi(xs);
        *joy_y_ptr = atoi(ys);
    }

    httpd_resp_send(req,"OK",HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* ================= Data interface ================= */

esp_err_t data_handler(httpd_req_t *req)
{
    char json[64];

    float angle = web_angle;
    float speed = web_speed;

    sprintf(json,
    "{\"angle\":%.4f,\"speed\":%.4f}",
    angle,
    speed);

    httpd_resp_set_type(req,"application/json");
    httpd_resp_send(req,json,HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

/* ================= Start Webserver ================= */

void start_webserver(volatile int *jx, volatile int *jy)
{
    joy_x_ptr = jx;
    joy_y_ptr = jy;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t root = {"/",HTTP_GET,root_handler,NULL};
        httpd_uri_t joy  = {"/joy",HTTP_GET,joy_handler,NULL};
        httpd_uri_t data = {"/data",HTTP_GET,data_handler,NULL};

        httpd_register_uri_handler(server,&root);
        httpd_register_uri_handler(server,&joy);
        httpd_register_uri_handler(server,&data);
    }
}