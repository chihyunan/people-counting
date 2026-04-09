#include <WiFi.h>
#include <WebServer.h>
#include "wifi_ap.h"

static WebServer server(80);

static int *g_inRoom = nullptr;
static unsigned long *g_entered = nullptr;
static unsigned long *g_exited = nullptr;

static void handleIndex() {
  server.send(200, "text/html",
               "<!DOCTYPE html><html><head><meta charset=utf-8>"
               "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
               "<title>Counts</title>"
               "<style>"
               "body{font-family:system-ui,sans-serif;margin:0;padding:1.5rem;background:#111;color:#eee}"
               "h1{text-align:center;font-size:1.1rem;color:#888;margin-bottom:1.5rem}"
               ".row{display:flex;justify-content:space-around;gap:1rem;max-width:28rem;margin:0 auto}"
               ".card{flex:1;background:#222;border-radius:12px;padding:1.25rem;text-align:center}"
               ".card h2{margin:0 0 .5rem;font-size:.85rem;text-transform:uppercase;letter-spacing:.06em}"
               ".card.enter h2{color:#6c6}"
               ".card.exit h2{color:#c96}"
               ".num{font-size:clamp(2rem,12vw,3.5rem);font-weight:700;line-height:1.2}"
               ".sub{text-align:center;margin-top:1rem;font-size:.85rem;color:#666}"
               "</style></head><body>"
               "<h1>In room: <span id=ir>&hellip;</span></h1>"
               "<div class=row>"
               "<div class=\"card enter\"><h2>Enter</h2><p class=num id=en>&hellip;</p></div>"
               "<div class=\"card exit\"><h2>Exit</h2><p class=num id=ex>&hellip;</p></div>"
               "</div>"
               "<p class=sub>/json</p>"
               "<script>"
               "async function run(){const ir=document.getElementById('ir');"
               "const en=document.getElementById('en');const ex=document.getElementById('ex');"
               "for(;;){try{const j=await(await fetch('/json',{cache:'no-store'})).json();"
               "ir.textContent=j.inRoom;en.textContent=j.entered;ex.textContent=j.exited;}"
               "catch(e){ir.textContent=en.textContent=ex.textContent='?';"
               "await new Promise(r=>setTimeout(r,100));}}}"
               "run();</script></body></html>");
}

static void handleJson() {
  if (!g_inRoom || !g_entered || !g_exited) {
    server.send(500, "application/json", "{}");
    return;
  }
  char body[96];
  snprintf(body, sizeof(body), "{\"inRoom\":%d,\"entered\":%lu,\"exited\":%lu}", *g_inRoom,
           *g_entered, *g_exited);
  server.send(200, "application/json", body);
}

static void handleState() {
  if (!g_inRoom || !g_entered || !g_exited) {
    server.send(500, "text/plain", "error\n");
    return;
  }
  char line[128];
  snprintf(line, sizeof(line), "inRoom=%d entered=%lu exited=%lu\n", *g_inRoom, *g_entered,
           *g_exited);
  server.send(200, "text/plain", line);
}

void wifiBegin(const char *apSsid, const char *apPass, int *inRoom, unsigned long *entered,
               unsigned long *exited) {
  g_inRoom = inRoom;
  g_entered = entered;
  g_exited = exited;

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPass);

  Serial.print(F("[wifi] AP "));
  Serial.println(apSsid);
  Serial.print(F("[wifi] http://"));
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleIndex);
  server.on("/json", HTTP_GET, handleJson);
  server.on("/state", HTTP_GET, handleState);
  server.begin();
}

void wifiLoop() { server.handleClient(); }
