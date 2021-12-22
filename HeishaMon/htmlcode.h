static const char webHeader[] PROGMEM  =
  "<!DOCTYPE html>"
  "<html>"
  "<title>Heisha monitor</title>"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";

static const char refreshMeta[] PROGMEM = "<meta http-equiv=\"refresh\" content=\"5; url=/\" />";
static const char webBodyStart[] PROGMEM =
  "<body>"
  "<button class=\"w3-button w3-red w3-xlarge w3-left\" onclick=\"openLeftMenu()\">&#9776;</button>"
  "<header class=\"w3-container w3-card w3-theme\"><h1>Heisha monitor</h1></header>";

static const char webFooter[] PROGMEM  = "</body></html>";
static const char menuJS[] PROGMEM =
  "<script>"
  "  function openLeftMenu() {"
  "   var x = document.getElementById(\"leftMenu\");"
  "   if (x.style.display === \"none\") {"
  "     x.style.display = \"block\";"
  "   } else {"
  "     x.style.display = \"none\";"
  "   }"
  " }"
  "</script>";

static const char websocketJS[] PROGMEM =
  "<script>"
  "  var bConnected = false;"
  "  function startWebsockets() {"
  "    if(typeof MozWebSocket != \"undefined\") {"
  "      oWebsocket = new MozWebSocket(\"ws://\" + location.host + \":81\");"
  "    } else if(typeof WebSocket != \"undefined\") {"
  "      /* The characters after the trailing slash are needed for a wierd IE 10 bug */"
  "      oWebsocket = new WebSocket(\"ws://\" + location.host + \":81/ws\");"
  "    }"
  ""
  "    if(oWebsocket) {"
  "      oWebsocket.onopen = function(evt) {"
  "        bConnected = true;"
  "      };"
  ""
  "      oWebsocket.onclose = function(evt) {"
  "        console.log('onclose: ' + evt);"
  "      };"
  ""
  "      oWebsocket.onerror = function(evt) {"
  "        console.log('onerror: ' + evt);"
  "      };"
  ""
  "      oWebsocket.onmessage = function(evt) {"
  "        let obj = document.getElementById(\"cli\");"
  "        let chk = document.getElementById(\"autoscroll\");"
  "        obj.value += evt.data + \"\\n\";"
  "        if(chk.checked) {"
  "          obj.scrollTop = obj.scrollHeight;"
  "        }"
  "      }"
  "    }"
  "  }"
  "</script>";

static const char refreshJS[] PROGMEM =
  "<script>"
  " document.body.onload=function() {"
  "    refreshTable();"
  "    openTable('Heatpump');"
  "    document.getElementById(\"cli\").value = \"\";"
  "    startWebsockets();"
  " };"
  " function loadContent(id, url, func) {"
  "   var xhr = new XMLHttpRequest();"
  "   xhr.open('GET', url, true);"
  "   xhr.send();"
  "   xhr.onload = function() {"
  "     if(xhr.status == 200) {"
  "       let obj = document.getElementById(id);"
  "       if(obj) {"
  "         obj.innerHTML = xhr.responseText;"
  "         func();"
  "       }"
  "     }"
  "   }"
  " }"
  " function refreshTable(){"
  "   loadContent('heishavalues', '/tablerefresh', function(){setTimeout(refreshTable, 30000);});"
  "   loadContent('dallasvalues', '/tablerefresh?1wire', function(){});"
  "   loadContent('s0values', '/tablerefresh?s0', function(){});"
  " }"
  "</script>";

static const char selectJS[] PROGMEM =
  "<script>"
  "function openTable(tableName) {"
  "  var i;"
  "  var x = document.getElementsByClassName(\"heishatable\");"
  "  for (i = 0; i < x.length; i++) {"
  "    x[i].style.display = \"none\";"
  "  }"
  "  document.getElementById(tableName).style.display = \"block\";"
  "}"
  "</script>";

static const char settingsJS[] PROGMEM =
  "<script type=\"text/javascript\">"
  "    function ShowHideDallasTable(dallasEnabled) {"
  "        var dallassettings = document.getElementById(\"dallassettings\");"
  "        dallassettings.style.display = dallasEnabled.checked ? \"table\" : \"none\";"
  "    }"
  "    function ShowHideS0Table(s0enabled) {"
  "        var s0settings = document.getElementById(\"s0settings\");"
  "        s0settings.style.display = s0enabled.checked ? \"table\" : \"none\";"
  "    }"
  "    function changeMinWatt(port) {"
  "        var ppkwh = document.getElementById('s0_ppkwh_'+port).value;"
  "        var interval = document.getElementById('s0_interval_'+port).value;"
  "        document.getElementById('s0_minwatt_'+port).innerHTML = Math.round((3600 * 1000 / ppkwh) / interval);"
  "    }"
  "</script>";

/*static const char heatingCurveJS[] PROGMEM =
  "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js\"></script>"
  "<script>"
  " $( document ).ready(function() {"
  "    $('#hcoh,#hcol,#hcth,#hctl').change(ChangeHeatingCurve);"
  "    ChangeHeatingCurve();"
  " });"
  " function ChangeHeatingCurve(){"
  "    var inputHcthValue = $('#hcth').val();"
  "    var inputHctlValue = $('#hctl').val();"
  "    var inputHcohValue = $('#hcoh').val();"
  "    var inputHcolValue = $('#hcol').val();"
  "    "
  "    $('#graph-hcth').text(inputHcthValue);"
  "    $('#graph-hctl').text(inputHctlValue);"
  "    $('#graph-hcoh').text(inputHcohValue);"
  "    $('#graph-hcol').text(inputHcolValue);"
  "    "
  "    var tableHTML = '';"
  "    for (i = 15; i > -21; i--) {"
  "        tableHTML += '<tr><td>';"
  "        tableHTML += i;"
  "        tableHTML += '</td><td>';"
  "        if (i > inputHcohValue) {"
  "            tableHTML += '<input class=\"w3-input w3-border-0\" type=\"text\" name=\"lookup'+(i+20)+'\" value=\"'+inputHctlValue+'\" readonly>';"
  "        } else if (i < inputHcolValue) {"
  "            tableHTML += '<input class=\"w3-input w3-border-0\" type=\"text\" name=\"lookup'+(i+20)+'\" value=\"'+inputHcthValue+'\" readonly>';"
  "        } else {"
  "            var temperature = ((inputHctlValue * 1.0) + (((inputHcthValue - inputHctlValue) / (inputHcohValue - inputHcolValue)) * (inputHcohValue - i)));"
  "            temperature = temperature.toFixed(0);"
  "            tableHTML += '<input class=\"w3-input w3-border-0\" type=\"text\" name=\"lookup'+(i+20)+'\" value=\"'+temperature+'\" readonly>';"
  "        }"
  "        tableHTML += '</td></tr>';"
  "    }"
  "    $(\"#heatcurvevalues\").html(tableHTML);"
  " }"
  "</script>";*/

static const char webBodyRoot1[] PROGMEM =
  "<div class=\"w3-sidebar w3-bar-block w3-card w3-animate-left\" style=\"display:none\" id=\"leftMenu\">"
  "<a href=\"/reboot\" class=\"w3-bar-item w3-button\">Reboot</a>"
  "<a href=\"/firmware\" class=\"w3-bar-item w3-button\">Firmware</a>"
  "<a href=\"/settings\" class=\"w3-bar-item w3-button\">Settings</a>"
  "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>"
  "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>"
  "<hr><div class=\"w3-text-grey\">Version: ";

static const char webBodyRoot2[] PROGMEM =
  "<br><a href=\"https://github.com/Egyras/HeishaMon\">Heishamon software</a></div><hr></div>"
  "<div class=\"w3-bar w3-red\">"
  "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('Heatpump')\">Heatpump</button>";

static const char webBodyRootDallasTab[] PROGMEM = "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('Dallas')\">Dallas 1-wire</button>";
static const char webBodyRootS0Tab[] PROGMEM = "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('S0')\">S0 kWh meters</button>";

static const char webBodyRootConsoleTab[] PROGMEM = "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('Console')\">Console</button>";

static const char webBodyEndDiv[] PROGMEM = "</div>";

static const char webBodyRootStatusWifi[] PROGMEM =   "<div class=\"w3-container w3-left\"><br>Wifi signal: ";
static const char webBodyRootStatusMemory[] PROGMEM =   "%<br>Memory free: ";
static const char webBodyRootStatusReceived[] PROGMEM =  "%<br>Correct received data: ";
static const char webBodyRootStatusReconnects[] PROGMEM =  "%<br>MQTT reconnects: ";
static const char webBodyRootStatusUptime[] PROGMEM =   "<br>Uptime: ";

static const char webBodyRootHeatpumpValues[] PROGMEM =
  "<div id=\"Heatpump\" class=\"w3-container w3-center heishatable\">"
  "<h2>Current heatpump values</h2>"
  "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Topic</th><th>Name</th><th>Value</th><th>Description</th></tr></thead><tbody id=\"heishavalues\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";

static const char webBodyRootDallasValues[] PROGMEM =
  "<div id=\"Dallas\" class=\"w3-container w3-center heishatable\" style=\"display:none\">"
  "<h2>Current Dallas 1-wire values</h2>"
  "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Sensor</th><th>Temperature</th></tr></thead><tbody id=\"dallasvalues\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";

static const char webBodyRootS0Values[] PROGMEM =
  "<div id=\"S0\" class=\"w3-container w3-center heishatable\" style=\"display:none\">"
  "<h2>Current S0 kWh meters values</h2>"
  "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>S0 port</th><th>Watt</th><th>Watthour</th><th>WatthourTotal</th><th>Pulse quality</th><th>Average pulse width</th></tr></thead><tbody id=\"s0values\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";

static const char webBodyRootConsole[] PROGMEM =
  "<div id=\"Console\" class=\"w3-container w3-center heishatable\">"
  "<h2>Console output</h2>"
  "<textarea id=\"cli\" disabled></textarea><br /><input type=\"checkbox\" id=\"autoscroll\" checked=\"checked\">Enable autoscroll</div>";

static const char webBodyFactoryResetWarning[] PROGMEM =
  "<div class=\"w3-container w3-center\">"
  "<p>Removing configuration. To reconfigure please connect to WiFi hotspot after reset.</p>"
  "</div>";

static const char webBodyRebootWarning[] PROGMEM =
  "<div class=\"w3-container w3-center\">"
  "<p>Rebooting</p>"
  "</div>";

static const char webBodySettingsNewWifiWarning[] PROGMEM =
  "<div class=\"w3-container w3-center\">"
  "<p><b>Reconfiguring WiFi</b><br /><br />"
  "Trying to connect to the new AP<br /><br />"
  "The Heishamon-Setup hotspot will<br />automatically be brought<br />down when this succeeds<br /><br />"
  "This page automatically<br />redirect to home in a few seconds</p>"
  "</div>";

static const char webBodySettingsResetPasswordWarning[] PROGMEM =
  "<div class=\"w3-container w3-center\">"
  "<p><b>Wrong password</b><br /><br />"
  "Do a factory reset to reset it to<br />the its default password: heisha</p>"
  "</div>";

static const char webBodySettingsSaveMessage[] PROGMEM =
  "<div class=\"w3-container w3-center\">"
  "<p><b>Configuration saved</b><br /><br />"
  "Rebooting</p>"
  "</div>";

static const char webBodySettings1[] PROGMEM =
  "<div class=\"w3-sidebar w3-bar-block w3-card w3-animate-left\" style=\"display:none\" id=\"leftMenu\">"
  "<a href=\"/\" class=\"w3-bar-item w3-button\">Home</a>"
  "<a href=\"/reboot\" class=\"w3-bar-item w3-button\">Reboot</a>"
  "<a href=\"/firmware\" class=\"w3-bar-item w3-button\">Firmware</a>"
  "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>"
  "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>"
  "</div>";

static const char webBodySmartcontrol1[] PROGMEM =
  "<div class=\"w3-sidebar w3-bar-block w3-card w3-animate-left\" style=\"display:none\" id=\"leftMenu\">"
  "<a href=\"/\" class=\"w3-bar-item w3-button\">Home</a>"
  "<a href=\"/reboot\" class=\"w3-bar-item w3-button\">Reboot</a>"
  "<a href=\"/firmware\" class=\"w3-bar-item w3-button\">Firmware</a>"
  "<a href=\"/settings\" class=\"w3-bar-item w3-button\">Settings</a>"
  "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>"
  "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>"
  "</div>";

static const char webBodySmartcontrol2[] PROGMEM =
  "<div class=\"w3-bar w3-red\">"
  "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('heatingcurve')\">Heating Curve</button>"
  //  "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('others')\">Others</button>"
  "</div>";

static const char webBodySmartcontrolHeatingcurve1[] PROGMEM =
  "<div id=\"heatingcurve\" class=\"w3-container w3-center heishatable\">"
  "<h2>Heating curve setting</h2>";

static const char webBodySmartcontrolHeatingcurve2[] PROGMEM =
  "<br><br>"
  "<table class=\"w3-table-all\">"
  "<thead>"
  "<tr class=\"w3-red\">"
  "<th>Outside Temperature</th>"
  "<th>Target Setpoint</th>"
  "</tr>"
  "</thead>"
  "<tbody id=\"heatcurvevalues\">"
  "<tr>"
  "<td>...Loading...</td>"
  "<td></td>"
  "</tr>"
  "</tbody>"
  "</table>";

static const char webBodySmartcontrolHeatingcurveSVG[] PROGMEM =
  "<svg version=\"1.2\" width=\"500\" height=\"450\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" class=\"graph\">"
  "<g style=\"stroke:black;stroke-dasharray: 5;stroke-width: 0.5;\">"
  "<line x1=\"100\" x2=\"500\" y1=\"0\" y2=\"0\"></line>"
  "<line x1=\"100\" x2=\"500\" y1=\"133\" y2=\"133\"></line>"
  "<line x1=\"100\" x2=\"500\" y1=\"267\" y2=\"267\"></line>"
  "<line x1=\"100\" x2=\"500\" y1=\"400\" y2=\"400\"></line>"
  "<line x1=\"100\" x2=\"100\" y1=\"0\" y2=\"400\"></line>"
  "<line x1=\"233\" x2=\"233\" y1=\"0\" y2=\"400\"></line>"
  "<line x1=\"367\" x2=\"367\" y1=\"0\" y2=\"400\"></line>"
  "<line x1=\"500\" x2=\"500\" y1=\"0\" y2=\"400\"></line>"
  "</g>"
  "<g style=\"text-anchor: middle;\">"
  "<text x=\"100\" y=\"420\">-20</text>"
  "<text x=\"233\" y=\"420\" id=\"graph-hcol\">-20</text>"
  "<text x=\"367\" y=\"420\" id=\"graph-hcoh\">15</text>"
  "<text x=\"490\" y=\"420\">15</text>"
  "</g>"
  "<g style=\"text-anchor: end;text-anchor: middle;\">"
  "<text x=\"80\" y=\"0 \" style=\"alignment-baseline:hanging\">60</text>"
  "<text x=\"80\" y=\"133 \" id=\"graph-hcth\" style=\"alignment-baseline:hanging\">60</text>"
  "<text x=\"80\" y=\"267 \" id=\"graph-hctl\" style=\"alignment-baseline:hanging\">20</text>"
  "<text x=\"80\" y=\"400 \" style=\"alignment-baseline:hanging\">20</text>"
  "</g>"
  "<g style=\"text-anchor: middle;\">"
  "<text x=\"50%\" y=\"450\">Outside temperature</text>"
  "</g>"
  "<g style=\"text-anchor: middle;\">"
  "<text x=\"15\" y=\"200\" transform=\"rotate(-90,15,200)\">Target setpoint</text>"
  "</g>"
  "<polyline style=\"stroke:green;stroke-width: 3;fill: none;\" points=\"100,133 233,133 367,267 500,267 \"/>"
  "</svg>";

//static const char webBodySmartcontrolOtherexample[] PROGMEM =
//  "<div id=\"others\" class=\"w3-container w3-center heishatable\" style=\"display:none\">"
//  "<h2>Other example</h2>";

static const char webCSS[] PROGMEM =
  "<style>"
  "/* W3.CSS 4.15 December 2020 by Jan Egil and Borge Refsnes */"
  "html{box-sizing:border-box}*,*:before,*:after{box-sizing:inherit}"
  "/* Extract from normalize.css by Nicolas Gallagher and Jonathan Neal git.io/normalize */"
  "html{-ms-text-size-adjust:100%;-webkit-text-size-adjust:100%}body{margin:0}"
  "article,aside,details,figcaption,figure,footer,header,main,menu,nav,section{display:block}summary{display:list-item}"
  "audio,canvas,progress,video{display:inline-block}progress{vertical-align:baseline}"
  "audio:not([controls]){display:none;height:0}[hidden],template{display:none}"
  "a{background-color:transparent}a:active,a:hover{outline-width:0}"
  "abbr[title]{border-bottom:none;text-decoration:underline;text-decoration:underline dotted}"
  "b,strong{font-weight:bolder}dfn{font-style:italic}mark{background:#ff0;color:#000}"
  "small{font-size:80%}sub,sup{font-size:75%;line-height:0;position:relative;vertical-align:baseline}"
  "sub{bottom:-0.25em}sup{top:-0.5em}figure{margin:1em 40px}img{border-style:none}"
  "code,kbd,pre,samp{font-family:monospace,monospace;font-size:1em}hr{box-sizing:content-box;height:0;overflow:visible}"
  "button,input,select,textarea,optgroup{font:inherit;margin:0}optgroup{font-weight:bold}"
  "button,input{overflow:visible}button,select{text-transform:none}"
  "button,[type=button],[type=reset],[type=submit]{-webkit-appearance:button}"
  "button::-moz-focus-inner,[type=button]::-moz-focus-inner,[type=reset]::-moz-focus-inner,[type=submit]::-moz-focus-inner{border-style:none;padding:0}"
  "button:-moz-focusring,[type=button]:-moz-focusring,[type=reset]:-moz-focusring,[type=submit]:-moz-focusring{outline:1px dotted ButtonText}"
  "fieldset{border:1px solid #c0c0c0;margin:0 2px;padding:.35em .625em .75em}"
  "legend{color:inherit;display:table;max-width:100%;padding:0;white-space:normal}textarea{overflow:auto}"
  "[type=checkbox],[type=radio]{padding:0}"
  "[type=number]::-webkit-inner-spin-button,[type=number]::-webkit-outer-spin-button{height:auto}"
  "[type=search]{-webkit-appearance:textfield;outline-offset:-2px}"
  "[type=search]::-webkit-search-decoration{-webkit-appearance:none}"
  "::-webkit-file-upload-button{-webkit-appearance:button;font:inherit}"
  "/* End extract */"
  "html,body{font-family:Verdana,sans-serif;font-size:15px;line-height:1.5}html{overflow-x:hidden}"
  "h1{font-size:36px}h2{font-size:30px}h3{font-size:24px}h4{font-size:20px}h5{font-size:18px}h6{font-size:16px}"
  ".w3-serif{font-family:serif}.w3-sans-serif{font-family:sans-serif}.w3-cursive{font-family:cursive}.w3-monospace{font-family:monospace}"
  "h1,h2,h3,h4,h5,h6{font-family:\"Segoe UI\",Arial,sans-serif;font-weight:400;margin:10px 0}.w3-wide{letter-spacing:4px}"
  "hr{border:0;border-top:1px solid #eee;margin:20px 0}"
  ".w3-image{max-width:100%;height:auto}img{vertical-align:middle}a{color:inherit}"
  ".w3-table,.w3-table-all{border-collapse:collapse;border-spacing:0;width:100%;display:table}.w3-table-all{border:1px solid #ccc}"
  ".w3-bordered tr,.w3-table-all tr{border-bottom:1px solid #ddd}.w3-striped tbody tr:nth-child(even){background-color:#f1f1f1}"
  ".w3-table-all tr:nth-child(odd){background-color:#fff}.w3-table-all tr:nth-child(even){background-color:#f1f1f1}"
  ".w3-hoverable tbody tr:hover,.w3-ul.w3-hoverable li:hover{background-color:#ccc}.w3-centered tr th,.w3-centered tr td{text-align:center}"
  ".w3-table td,.w3-table th,.w3-table-all td,.w3-table-all th{padding:8px 8px;display:table-cell;text-align:left;vertical-align:top}"
  ".w3-table th:first-child,.w3-table td:first-child,.w3-table-all th:first-child,.w3-table-all td:first-child{padding-left:16px}"
  ".w3-btn,.w3-button{border:none;display:inline-block;padding:8px 16px;vertical-align:middle;overflow:hidden;text-decoration:none;color:inherit;background-color:inherit;text-align:center;cursor:pointer;white-space:nowrap}"
  ".w3-btn:hover{box-shadow:0 8px 16px 0 rgba(0,0,0,0.2),0 6px 20px 0 rgba(0,0,0,0.19)}"
  ".w3-btn,.w3-button{-webkit-touch-callout:none;-webkit-user-select:none;-khtml-user-select:none;-moz-user-select:none;-ms-user-select:none;user-select:none}   "
  ".w3-disabled,.w3-btn:disabled,.w3-button:disabled{cursor:not-allowed;opacity:0.3}.w3-disabled *,:disabled *{pointer-events:none}"
  ".w3-btn.w3-disabled:hover,.w3-btn:disabled:hover{box-shadow:none}"
  ".w3-badge,.w3-tag{background-color:#000;color:#fff;display:inline-block;padding-left:8px;padding-right:8px;text-align:center}.w3-badge{border-radius:50%}"
  ".w3-ul{list-style-type:none;padding:0;margin:0}.w3-ul li{padding:8px 16px;border-bottom:1px solid #ddd}.w3-ul li:last-child{border-bottom:none}"
  ".w3-tooltip,.w3-display-container{position:relative}.w3-tooltip .w3-text{display:none}.w3-tooltip:hover .w3-text{display:inline-block}"
  ".w3-ripple:active{opacity:0.5}.w3-ripple{transition:opacity 0s}"
  ".w3-input{padding:8px;display:block;border:none;border-bottom:1px solid #ccc;width:100%}"
  ".w3-select{padding:9px 0;width:100%;border:none;border-bottom:1px solid #ccc}"
  ".w3-dropdown-click,.w3-dropdown-hover{position:relative;display:inline-block;cursor:pointer}"
  ".w3-dropdown-hover:hover .w3-dropdown-content{display:block}"
  ".w3-dropdown-hover:first-child,.w3-dropdown-click:hover{background-color:#ccc;color:#000}"
  ".w3-dropdown-hover:hover > .w3-button:first-child,.w3-dropdown-click:hover > .w3-button:first-child{background-color:#ccc;color:#000}"
  ".w3-dropdown-content{cursor:auto;color:#000;background-color:#fff;display:none;position:absolute;min-width:160px;margin:0;padding:0;z-index:1}"
  ".w3-check,.w3-radio{width:24px;height:24px;position:relative;top:6px}"
  ".w3-sidebar{height:100%;width:200px;background-color:#fff;position:fixed!important;z-index:1;overflow:auto}"
  ".w3-bar-block .w3-dropdown-hover,.w3-bar-block .w3-dropdown-click{width:100%}"
  ".w3-bar-block .w3-dropdown-hover .w3-dropdown-content,.w3-bar-block .w3-dropdown-click .w3-dropdown-content{min-width:100%}"
  ".w3-bar-block .w3-dropdown-hover .w3-button,.w3-bar-block .w3-dropdown-click .w3-button{width:100%;text-align:left;padding:8px 16px}"
  ".w3-main,#main{transition:margin-left .4s}"
  ".w3-modal{z-index:3;display:none;padding-top:100px;position:fixed;left:0;top:0;width:100%;height:100%;overflow:auto;background-color:rgb(0,0,0);background-color:rgba(0,0,0,0.4)}"
  ".w3-modal-content{margin:auto;background-color:#fff;position:relative;padding:0;outline:0;width:600px}"
  ".w3-bar{width:100%;overflow:hidden}.w3-center .w3-bar{display:inline-block;width:auto}"
  ".w3-bar .w3-bar-item{padding:8px 16px;float:left;width:auto;border:none;display:block;outline:0}"
  ".w3-bar .w3-dropdown-hover,.w3-bar .w3-dropdown-click{position:static;float:left}"
  ".w3-bar .w3-button{white-space:normal}"
  ".w3-bar-block .w3-bar-item{width:100%;display:block;padding:8px 16px;text-align:left;border:none;white-space:normal;float:none;outline:0}"
  ".w3-bar-block.w3-center .w3-bar-item{text-align:center}.w3-block{display:block;width:100%}"
  ".w3-responsive{display:block;overflow-x:auto}"
  ".w3-container:after,.w3-container:before,.w3-panel:after,.w3-panel:before,.w3-row:after,.w3-row:before,.w3-row-padding:after,.w3-row-padding:before,"
  ".w3-cell-row:before,.w3-cell-row:after,.w3-clear:after,.w3-clear:before,.w3-bar:before,.w3-bar:after{content:"";display:table;clear:both}"
  ".w3-col,.w3-half,.w3-third,.w3-twothird,.w3-threequarter,.w3-quarter{float:left;width:100%}"
  ".w3-col.s1{width:8.33333%}.w3-col.s2{width:16.66666%}.w3-col.s3{width:24.99999%}.w3-col.s4{width:33.33333%}"
  ".w3-col.s5{width:41.66666%}.w3-col.s6{width:49.99999%}.w3-col.s7{width:58.33333%}.w3-col.s8{width:66.66666%}"
  ".w3-col.s9{width:74.99999%}.w3-col.s10{width:83.33333%}.w3-col.s11{width:91.66666%}.w3-col.s12{width:99.99999%}"
  "@media (min-width:601px){.w3-col.m1{width:8.33333%}.w3-col.m2{width:16.66666%}.w3-col.m3,.w3-quarter{width:24.99999%}.w3-col.m4,.w3-third{width:33.33333%}"
  ".w3-col.m5{width:41.66666%}.w3-col.m6,.w3-half{width:49.99999%}.w3-col.m7{width:58.33333%}.w3-col.m8,.w3-twothird{width:66.66666%}"
  ".w3-col.m9,.w3-threequarter{width:74.99999%}.w3-col.m10{width:83.33333%}.w3-col.m11{width:91.66666%}.w3-col.m12{width:99.99999%}}"
  "@media (min-width:993px){.w3-col.l1{width:8.33333%}.w3-col.l2{width:16.66666%}.w3-col.l3{width:24.99999%}.w3-col.l4{width:33.33333%}"
  ".w3-col.l5{width:41.66666%}.w3-col.l6{width:49.99999%}.w3-col.l7{width:58.33333%}.w3-col.l8{width:66.66666%}"
  ".w3-col.l9{width:74.99999%}.w3-col.l10{width:83.33333%}.w3-col.l11{width:91.66666%}.w3-col.l12{width:99.99999%}}"
  ".w3-rest{overflow:hidden}.w3-stretch{margin-left:-16px;margin-right:-16px}"
  ".w3-content,.w3-auto{margin-left:auto;margin-right:auto}.w3-content{max-width:980px}.w3-auto{max-width:1140px}"
  ".w3-cell-row{display:table;width:100%}.w3-cell{display:table-cell}"
  ".w3-cell-top{vertical-align:top}.w3-cell-middle{vertical-align:middle}.w3-cell-bottom{vertical-align:bottom}"
  ".w3-hide{display:none!important}.w3-show-block,.w3-show{display:block!important}.w3-show-inline-block{display:inline-block!important}"
  "@media (max-width:1205px){.w3-auto{max-width:95%}}"
  "@media (max-width:600px){.w3-modal-content{margin:0 10px;width:auto!important}.w3-modal{padding-top:30px}"
  ".w3-dropdown-hover.w3-mobile .w3-dropdown-content,.w3-dropdown-click.w3-mobile .w3-dropdown-content{position:relative}	"
  ".w3-hide-small{display:none!important}.w3-mobile{display:block;width:100%!important}.w3-bar-item.w3-mobile,.w3-dropdown-hover.w3-mobile,.w3-dropdown-click.w3-mobile{text-align:center}"
  ".w3-dropdown-hover.w3-mobile,.w3-dropdown-hover.w3-mobile .w3-btn,.w3-dropdown-hover.w3-mobile .w3-button,.w3-dropdown-click.w3-mobile,.w3-dropdown-click.w3-mobile .w3-btn,.w3-dropdown-click.w3-mobile .w3-button{width:100%}}"
  "@media (max-width:768px){.w3-modal-content{width:500px}.w3-modal{padding-top:50px}}"
  "@media (min-width:993px){.w3-modal-content{width:900px}.w3-hide-large{display:none!important}.w3-sidebar.w3-collapse{display:block!important}}"
  "@media (max-width:992px) and (min-width:601px){.w3-hide-medium{display:none!important}}"
  "@media (max-width:992px){.w3-sidebar.w3-collapse{display:none}.w3-main{margin-left:0!important;margin-right:0!important}.w3-auto{max-width:100%}}"
  ".w3-top,.w3-bottom{position:fixed;width:100%;z-index:1}.w3-top{top:0}.w3-bottom{bottom:0}"
  ".w3-overlay{position:fixed;display:none;width:100%;height:100%;top:0;left:0;right:0;bottom:0;background-color:rgba(0,0,0,0.5);z-index:2}"
  ".w3-display-topleft{position:absolute;left:0;top:0}.w3-display-topright{position:absolute;right:0;top:0}"
  ".w3-display-bottomleft{position:absolute;left:0;bottom:0}.w3-display-bottomright{position:absolute;right:0;bottom:0}"
  ".w3-display-middle{position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);-ms-transform:translate(-50%,-50%)}"
  ".w3-display-left{position:absolute;top:50%;left:0%;transform:translate(0%,-50%);-ms-transform:translate(-0%,-50%)}"
  ".w3-display-right{position:absolute;top:50%;right:0%;transform:translate(0%,-50%);-ms-transform:translate(0%,-50%)}"
  ".w3-display-topmiddle{position:absolute;left:50%;top:0;transform:translate(-50%,0%);-ms-transform:translate(-50%,0%)}"
  ".w3-display-bottommiddle{position:absolute;left:50%;bottom:0;transform:translate(-50%,0%);-ms-transform:translate(-50%,0%)}"
  ".w3-display-container:hover .w3-display-hover{display:block}.w3-display-container:hover span.w3-display-hover{display:inline-block}.w3-display-hover{display:none}"
  ".w3-display-position{position:absolute}"
  ".w3-circle{border-radius:50%}"
  ".w3-round-small{border-radius:2px}.w3-round,.w3-round-medium{border-radius:4px}.w3-round-large{border-radius:8px}.w3-round-xlarge{border-radius:16px}.w3-round-xxlarge{border-radius:32px}"
  ".w3-row-padding,.w3-row-padding>.w3-half,.w3-row-padding>.w3-third,.w3-row-padding>.w3-twothird,.w3-row-padding>.w3-threequarter,.w3-row-padding>.w3-quarter,.w3-row-padding>.w3-col{padding:0 8px}"
  ".w3-container,.w3-panel{padding:0.01em 16px}.w3-panel{margin-top:16px;margin-bottom:16px}"
  ".w3-code,.w3-codespan{font-family:Consolas,\"courier new\";font-size:16px}"
  ".w3-code{width:auto;background-color:#fff;padding:8px 12px;border-left:4px solid #4CAF50;word-wrap:break-word}"
  ".w3-codespan{color:crimson;background-color:#f1f1f1;padding-left:4px;padding-right:4px;font-size:110%}"
  ".w3-card,.w3-card-2{box-shadow:0 2px 5px 0 rgba(0,0,0,0.16),0 2px 10px 0 rgba(0,0,0,0.12)}"
  ".w3-card-4,.w3-hover-shadow:hover{box-shadow:0 4px 10px 0 rgba(0,0,0,0.2),0 4px 20px 0 rgba(0,0,0,0.19)}"
  ".w3-spin{animation:w3-spin 2s infinite linear}@keyframes w3-spin{0%{transform:rotate(0deg)}100%{transform:rotate(359deg)}}"
  ".w3-animate-fading{animation:fading 10s infinite}@keyframes fading{0%{opacity:0}50%{opacity:1}100%{opacity:0}}"
  ".w3-animate-opacity{animation:opac 0.8s}@keyframes opac{from{opacity:0} to{opacity:1}}"
  ".w3-animate-top{position:relative;animation:animatetop 0.4s}@keyframes animatetop{from{top:-300px;opacity:0} to{top:0;opacity:1}}"
  ".w3-animate-left{position:relative;animation:animateleft 0.4s}@keyframes animateleft{from{left:-300px;opacity:0} to{left:0;opacity:1}}"
  ".w3-animate-right{position:relative;animation:animateright 0.4s}@keyframes animateright{from{right:-300px;opacity:0} to{right:0;opacity:1}}"
  ".w3-animate-bottom{position:relative;animation:animatebottom 0.4s}@keyframes animatebottom{from{bottom:-300px;opacity:0} to{bottom:0;opacity:1}}"
  ".w3-animate-zoom {animation:animatezoom 0.6s}@keyframes animatezoom{from{transform:scale(0)} to{transform:scale(1)}}"
  ".w3-animate-input{transition:width 0.4s ease-in-out}.w3-animate-input:focus{width:100%!important}"
  ".w3-opacity,.w3-hover-opacity:hover{opacity:0.60}.w3-opacity-off,.w3-hover-opacity-off:hover{opacity:1}"
  ".w3-opacity-max{opacity:0.25}.w3-opacity-min{opacity:0.75}"
  ".w3-greyscale-max,.w3-grayscale-max,.w3-hover-greyscale:hover,.w3-hover-grayscale:hover{filter:grayscale(100%)}"
  ".w3-greyscale,.w3-grayscale{filter:grayscale(75%)}.w3-greyscale-min,.w3-grayscale-min{filter:grayscale(50%)}"
  ".w3-sepia{filter:sepia(75%)}.w3-sepia-max,.w3-hover-sepia:hover{filter:sepia(100%)}.w3-sepia-min{filter:sepia(50%)}"
  ".w3-tiny{font-size:10px!important}.w3-small{font-size:12px!important}.w3-medium{font-size:15px!important}.w3-large{font-size:18px!important}"
  ".w3-xlarge{font-size:24px!important}.w3-xxlarge{font-size:36px!important}.w3-xxxlarge{font-size:48px!important}.w3-jumbo{font-size:64px!important}"
  ".w3-left-align{text-align:left!important}.w3-right-align{text-align:right!important}.w3-justify{text-align:justify!important}.w3-center{text-align:center!important}"
  ".w3-border-0{border:0!important}.w3-border{border:1px solid #ccc!important}"
  ".w3-border-top{border-top:1px solid #ccc!important}.w3-border-bottom{border-bottom:1px solid #ccc!important}"
  ".w3-border-left{border-left:1px solid #ccc!important}.w3-border-right{border-right:1px solid #ccc!important}"
  ".w3-topbar{border-top:6px solid #ccc!important}.w3-bottombar{border-bottom:6px solid #ccc!important}"
  ".w3-leftbar{border-left:6px solid #ccc!important}.w3-rightbar{border-right:6px solid #ccc!important}"
  ".w3-section,.w3-code{margin-top:16px!important;margin-bottom:16px!important}"
  ".w3-margin{margin:16px!important}.w3-margin-top{margin-top:16px!important}.w3-margin-bottom{margin-bottom:16px!important}"
  ".w3-margin-left{margin-left:16px!important}.w3-margin-right{margin-right:16px!important}"
  ".w3-padding-small{padding:4px 8px!important}.w3-padding{padding:8px 16px!important}.w3-padding-large{padding:12px 24px!important}"
  ".w3-padding-16{padding-top:16px!important;padding-bottom:16px!important}.w3-padding-24{padding-top:24px!important;padding-bottom:24px!important}"
  ".w3-padding-32{padding-top:32px!important;padding-bottom:32px!important}.w3-padding-48{padding-top:48px!important;padding-bottom:48px!important}"
  ".w3-padding-64{padding-top:64px!important;padding-bottom:64px!important}"
  ".w3-padding-top-64{padding-top:64px!important}.w3-padding-top-48{padding-top:48px!important}"
  ".w3-padding-top-32{padding-top:32px!important}.w3-padding-top-24{padding-top:24px!important}"
  ".w3-left{float:left!important}.w3-right{float:right!important}"
  ".w3-button:hover{color:#000!important;background-color:#ccc!important}"
  ".w3-transparent,.w3-hover-none:hover{background-color:transparent!important}"
  ".w3-hover-none:hover{box-shadow:none!important}"
  "/* Colors */"
  ".w3-amber,.w3-hover-amber:hover{color:#000!important;background-color:#ffc107!important}"
  ".w3-aqua,.w3-hover-aqua:hover{color:#000!important;background-color:#00ffff!important}"
  ".w3-blue,.w3-hover-blue:hover{color:#fff!important;background-color:#2196F3!important}"
  ".w3-light-blue,.w3-hover-light-blue:hover{color:#000!important;background-color:#87CEEB!important}"
  ".w3-brown,.w3-hover-brown:hover{color:#fff!important;background-color:#795548!important}"
  ".w3-cyan,.w3-hover-cyan:hover{color:#000!important;background-color:#00bcd4!important}"
  ".w3-blue-grey,.w3-hover-blue-grey:hover,.w3-blue-gray,.w3-hover-blue-gray:hover{color:#fff!important;background-color:#607d8b!important}"
  ".w3-green,.w3-hover-green:hover{color:#fff!important;background-color:#4CAF50!important}"
  ".w3-light-green,.w3-hover-light-green:hover{color:#000!important;background-color:#8bc34a!important}"
  ".w3-indigo,.w3-hover-indigo:hover{color:#fff!important;background-color:#3f51b5!important}"
  ".w3-khaki,.w3-hover-khaki:hover{color:#000!important;background-color:#f0e68c!important}"
  ".w3-lime,.w3-hover-lime:hover{color:#000!important;background-color:#cddc39!important}"
  ".w3-orange,.w3-hover-orange:hover{color:#000!important;background-color:#ff9800!important}"
  ".w3-deep-orange,.w3-hover-deep-orange:hover{color:#fff!important;background-color:#ff5722!important}"
  ".w3-pink,.w3-hover-pink:hover{color:#fff!important;background-color:#e91e63!important}"
  ".w3-purple,.w3-hover-purple:hover{color:#fff!important;background-color:#9c27b0!important}"
  ".w3-deep-purple,.w3-hover-deep-purple:hover{color:#fff!important;background-color:#673ab7!important}"
  ".w3-red,.w3-hover-red:hover{color:#fff!important;background-color:#f44336!important}"
  ".w3-sand,.w3-hover-sand:hover{color:#000!important;background-color:#fdf5e6!important}"
  ".w3-teal,.w3-hover-teal:hover{color:#fff!important;background-color:#009688!important}"
  ".w3-yellow,.w3-hover-yellow:hover{color:#000!important;background-color:#ffeb3b!important}"
  ".w3-white,.w3-hover-white:hover{color:#000!important;background-color:#fff!important}"
  ".w3-black,.w3-hover-black:hover{color:#fff!important;background-color:#000!important}"
  ".w3-grey,.w3-hover-grey:hover,.w3-gray,.w3-hover-gray:hover{color:#000!important;background-color:#9e9e9e!important}"
  ".w3-light-grey,.w3-hover-light-grey:hover,.w3-light-gray,.w3-hover-light-gray:hover{color:#000!important;background-color:#f1f1f1!important}"
  ".w3-dark-grey,.w3-hover-dark-grey:hover,.w3-dark-gray,.w3-hover-dark-gray:hover{color:#fff!important;background-color:#616161!important}"
  ".w3-pale-red,.w3-hover-pale-red:hover{color:#000!important;background-color:#ffdddd!important}"
  ".w3-pale-green,.w3-hover-pale-green:hover{color:#000!important;background-color:#ddffdd!important}"
  ".w3-pale-yellow,.w3-hover-pale-yellow:hover{color:#000!important;background-color:#ffffcc!important}"
  ".w3-pale-blue,.w3-hover-pale-blue:hover{color:#000!important;background-color:#ddffff!important}"
  ".w3-text-amber,.w3-hover-text-amber:hover{color:#ffc107!important}"
  ".w3-text-aqua,.w3-hover-text-aqua:hover{color:#00ffff!important}"
  ".w3-text-blue,.w3-hover-text-blue:hover{color:#2196F3!important}"
  ".w3-text-light-blue,.w3-hover-text-light-blue:hover{color:#87CEEB!important}"
  ".w3-text-brown,.w3-hover-text-brown:hover{color:#795548!important}"
  ".w3-text-cyan,.w3-hover-text-cyan:hover{color:#00bcd4!important}"
  ".w3-text-blue-grey,.w3-hover-text-blue-grey:hover,.w3-text-blue-gray,.w3-hover-text-blue-gray:hover{color:#607d8b!important}"
  ".w3-text-green,.w3-hover-text-green:hover{color:#4CAF50!important}"
  ".w3-text-light-green,.w3-hover-text-light-green:hover{color:#8bc34a!important}"
  ".w3-text-indigo,.w3-hover-text-indigo:hover{color:#3f51b5!important}"
  ".w3-text-khaki,.w3-hover-text-khaki:hover{color:#b4aa50!important}"
  ".w3-text-lime,.w3-hover-text-lime:hover{color:#cddc39!important}"
  ".w3-text-orange,.w3-hover-text-orange:hover{color:#ff9800!important}"
  ".w3-text-deep-orange,.w3-hover-text-deep-orange:hover{color:#ff5722!important}"
  ".w3-text-pink,.w3-hover-text-pink:hover{color:#e91e63!important}"
  ".w3-text-purple,.w3-hover-text-purple:hover{color:#9c27b0!important}"
  ".w3-text-deep-purple,.w3-hover-text-deep-purple:hover{color:#673ab7!important}"
  ".w3-text-red,.w3-hover-text-red:hover{color:#f44336!important}"
  ".w3-text-sand,.w3-hover-text-sand:hover{color:#fdf5e6!important}"
  ".w3-text-teal,.w3-hover-text-teal:hover{color:#009688!important}"
  ".w3-text-yellow,.w3-hover-text-yellow:hover{color:#d2be0e!important}"
  ".w3-text-white,.w3-hover-text-white:hover{color:#fff!important}"
  ".w3-text-black,.w3-hover-text-black:hover{color:#000!important}"
  ".w3-text-grey,.w3-hover-text-grey:hover,.w3-text-gray,.w3-hover-text-gray:hover{color:#757575!important}"
  ".w3-text-light-grey,.w3-hover-text-light-grey:hover,.w3-text-light-gray,.w3-hover-text-light-gray:hover{color:#f1f1f1!important}"
  ".w3-text-dark-grey,.w3-hover-text-dark-grey:hover,.w3-text-dark-gray,.w3-hover-text-dark-gray:hover{color:#3a3a3a!important}"
  ".w3-border-amber,.w3-hover-border-amber:hover{border-color:#ffc107!important}"
  ".w3-border-aqua,.w3-hover-border-aqua:hover{border-color:#00ffff!important}"
  ".w3-border-blue,.w3-hover-border-blue:hover{border-color:#2196F3!important}"
  ".w3-border-light-blue,.w3-hover-border-light-blue:hover{border-color:#87CEEB!important}"
  ".w3-border-brown,.w3-hover-border-brown:hover{border-color:#795548!important}"
  ".w3-border-cyan,.w3-hover-border-cyan:hover{border-color:#00bcd4!important}"
  ".w3-border-blue-grey,.w3-hover-border-blue-grey:hover,.w3-border-blue-gray,.w3-hover-border-blue-gray:hover{border-color:#607d8b!important}"
  ".w3-border-green,.w3-hover-border-green:hover{border-color:#4CAF50!important}"
  ".w3-border-light-green,.w3-hover-border-light-green:hover{border-color:#8bc34a!important}"
  ".w3-border-indigo,.w3-hover-border-indigo:hover{border-color:#3f51b5!important}"
  ".w3-border-khaki,.w3-hover-border-khaki:hover{border-color:#f0e68c!important}"
  ".w3-border-lime,.w3-hover-border-lime:hover{border-color:#cddc39!important}"
  ".w3-border-orange,.w3-hover-border-orange:hover{border-color:#ff9800!important}"
  ".w3-border-deep-orange,.w3-hover-border-deep-orange:hover{border-color:#ff5722!important}"
  ".w3-border-pink,.w3-hover-border-pink:hover{border-color:#e91e63!important}"
  ".w3-border-purple,.w3-hover-border-purple:hover{border-color:#9c27b0!important}"
  ".w3-border-deep-purple,.w3-hover-border-deep-purple:hover{border-color:#673ab7!important}"
  ".w3-border-red,.w3-hover-border-red:hover{border-color:#f44336!important}"
  ".w3-border-sand,.w3-hover-border-sand:hover{border-color:#fdf5e6!important}"
  ".w3-border-teal,.w3-hover-border-teal:hover{border-color:#009688!important}"
  ".w3-border-yellow,.w3-hover-border-yellow:hover{border-color:#ffeb3b!important}"
  ".w3-border-white,.w3-hover-border-white:hover{border-color:#fff!important}"
  ".w3-border-black,.w3-hover-border-black:hover{border-color:#000!important}"
  ".w3-border-grey,.w3-hover-border-grey:hover,.w3-border-gray,.w3-hover-border-gray:hover{border-color:#9e9e9e!important}"
  ".w3-border-light-grey,.w3-hover-border-light-grey:hover,.w3-border-light-gray,.w3-hover-border-light-gray:hover{border-color:#f1f1f1!important}"
  ".w3-border-dark-grey,.w3-hover-border-dark-grey:hover,.w3-border-dark-gray,.w3-hover-border-dark-gray:hover{border-color:#616161!important}"
  ".w3-border-pale-red,.w3-hover-border-pale-red:hover{border-color:#ffe7e7!important}.w3-border-pale-green,.w3-hover-border-pale-green:hover{border-color:#e7ffe7!important}"
  ".w3-border-pale-yellow,.w3-hover-border-pale-yellow:hover{border-color:#ffffcc!important}.w3-border-pale-blue,.w3-hover-border-pale-blue:hover{border-color:#e7ffff!important}"
  ".w3-theme {color:#fff !important; background-color:#f44336 !important}"
  ".w3-btn { margin-bottom:10px; }"
  ".heishatable { display: none; }"
  "#cli{ background: black; color: white; width: 100%; height: 400px!important; }"
  "</style>";


static const char changewifissidJS[] PROGMEM =
  "<script>"
  "function changewifissid() {"
  " var x = document.getElementById(\"wifi_ssid_select\").value;"
  " document.getElementById(\"wifi_ssid_id\").value = x;"
  "}"
  "</script>";

static const char populatescanwifiJS[] PROGMEM =
  "<script>"
  "var refreshWifiScan = function () {"
  " var selectList = document.getElementById('wifi_ssid_select');"
  " var request = new XMLHttpRequest();"
  " request.onreadystatechange = function(response) {"
  "  if (request.readyState === 4) {"
  "   if (request.status === 200) {"
  "      var jsonOptions = JSON.parse(request.responseText);"
  "      document.getElementById('wifi_ssid_select').innerText = null;"
  "      var defaultoption = document.createElement('option');"
  "      defaultoption.value = '';"
  "      defaultoption.text = 'Select SSID';"
  "      defaultoption.selected = true;"
  "      defaultoption.hidden = true;"
  "      selectList.appendChild(defaultoption);"
  "      jsonOptions.forEach(function(item) {"
  "        var option = document.createElement('option');"
  "        option.value = item.ssid;"
  "        option.text = item.ssid + \" - \" + item.rssi;"
  "        selectList.appendChild(option);"
  "      });"
  "       selectList.style.display = \"block\";"
  "     };"
  "  };"
  " };"
  " request.open('GET', '/wifiscan', true);"
  " request.send();"
  " setTimeout(refreshWifiScan,10000);"
  "};"
  "refreshWifiScan();"
  "</script>";


static const char settingsForm1[] PROGMEM =
  "<div class=\"w3-container w3-center\" id=\"loading_settings\">"
  "       <h2>Please wait, loading saved settings...</h2>"
  "</div>"
  "<div class=\"w3-container w3-center\" id=\"settings_form\" style=\"display:none\">"
  "  <h2>Settings</h2>"
  "  <form accept-charset=\"UTF-8\" action=\"/savesettings\" method=\"POST\">"
  "    <table style=\"width:100%\">"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Hostname:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"wifi_hostname\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Wifi SSID:</td>"
  "        <td style=\"text-align:left\">"
  "          <input list=\"available_ssid\" name=\"wifi_ssid\" id=\"wifi_ssid_id\" value=\"\">"
  "          <select id=\"wifi_ssid_select\" style=\"display:none\" onchange=\"changewifissid()\">"
  "            <option hidden selected value=\"\">Select SSID</option>"
  "          </select>"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Wifi password:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"password\" name=\"wifi_password\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Update username:</td>"
  "        <td style=\"text-align:left\">"
  "          <label name=\"username\">admin</label>"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Current update password:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"password\" name=\"current_ota_password\" value=\"\"> default password: \"heisha\""
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          New update password:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"password\" name=\"new_ota_password\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt topic base:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"mqtt_topic_base\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt server:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"mqtt_server\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt port:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" name=\"mqtt_port\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt username:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"mqtt_username\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt password:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"password\" name=\"mqtt_password\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          NTP servers (comma separated):</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"ntp_servers\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Timezone:</td>"
  "        <td style=\"text-align:left\">"
  "          <select name=\"timezone\">";

static const char settingsForm2[] PROGMEM =
  "          </select>"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          How often new values are collected from heatpump:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" name=\"waitTime\" value=\"\"> seconds (min 5 sec)"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          How often all heatpump values are retransmitted to MQTT broker:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" name=\"updateAllTime\" value=\"\"> seconds"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Listen only mode:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"listenonly\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Debug log to MQTT topic from start:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"logMqtt\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Debug log hexdump enable from start:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"logHexdump\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Debug log to serial1 (GPIO2):</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"logSerial1\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Emulate optional PCB:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"optionalPCB\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "    </table>"
  "    <table style=\"width:100%\">"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Use 1wire DS18b20:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" onclick=\"ShowHideDallasTable(this)\" name=\"use_1wire\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "    </table>"
  "    <table id=\"dallassettings\" style=\"display: none; width:100%\">"
  "      </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          How often new values are collected from 1wire:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" name=\"waitDallasTime\" value=\"\"> seconds (min 5 sec)"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          How often all 1wire values are retransmitted to MQTT broker:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" name=\"updataAllDallasTime\" value=\"\"> seconds"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          DS18b20 temperature resolution:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"radio\" id=\"9-bit\" name=\"dallasResolution\" value=\"9\"><label for=\"9-bit\"> 9-bit </label>"
  "          <input type=\"radio\" id=\"10-bit\" name=\"dallasResolution\" value=\"10\"><label for=\"10-bit\"> 10-bit </label>"
  "          <input type=\"radio\" id=\"11-bit\" name=\"dallasResolution\" value=\"11\"><label for=\"11-bit\"> 11-bit </label>"
  "          <input type=\"radio\" id=\"12-bit\" name=\"dallasResolution\" value=\"12\"><label for=\"12-bit\"> 12-bit </label>"
  "        </td>"
  "      </tr>"
  "    </table>"
  "    <table style=\"width:100%\">"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Use s0 kWh metering:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" onclick=\"ShowHideS0Table(this)\" name=\"use_s0\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "    </table>"
  "    <table id=\"s0settings\" style=\"display: none; width:100%\">"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 1 GPIO:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" name=\"s0_1_gpio\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 1 imp/kwh:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" id=\"s0_ppkwh_1\" onchange=\"changeMinWatt(1)\" name=\"s0_1_ppkwh\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 1 reporting interval during standby/low power usage:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" id=\"s0_interval_1\" onchange=\"changeMinWatt(1)\" name=\"s0_1_interval\" value=\"\"> seconds"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 1 minimal pulse width:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" id=\"s0_minpulsewidth_1\" name=\"s0_1_minpulsewidth\" value=\"\"> milliseconds"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 1 maximal pulse width:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" id=\"s0_maxpulsewidth_1\" name=\"s0_1_maxpulsewidth\" value=\"\"> milliseconds"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 1 standby/low power usage threshold:</td>"
  "        <td style=\"text-align:left\"><label id=\"s0_minwatt_1\"></label> Watt"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 2 GPIO:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" name=\"s0_2_gpio\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 2 imp/kwh:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" id=\"s0_ppkwh_2\" onchange=\"changeMinWatt(2)\" name=\"s0_2_ppkwh\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 2 reporting interval during standby/low power usage:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" id=\"s0_interval_2\" onchange=\"changeMinWatt(2)\" name=\"s0_2_interval\" value=\"\"> seconds"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 2 minimal pulse width:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" id=\"s0_minpulsewidth_2\" name=\"s0_2_minpulsewidth\" value=\"\"> milliseconds"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 2 maximal pulse width:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" id=\"s0_maxpulsewidth_2\" name=\"s0_2_maxpulsewidth\" value=\"\"> milliseconds"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">S0 port 2 standby/low power usage threshold:</td>"
  "        <td style=\"text-align:left\"><label id=\"s0_minwatt_2\"></label> Watt"
  "        </td>"
  "      </tr>"
  "    </table>"
  "    <br><br>"
  "    <input class=\"w3-green w3-button\" type=\"submit\" id=\"save_settings\" value=\"Save\">"
  "  </form>"
  "  <br><a href=\"/factoryreset\" class=\"w3-red w3-button\" onclick=\"return confirm('Are you sure?')\">Factory reset</a>"
  "</div>";

const char populategetsettingsJS[] PROGMEM =
  "<script>"
  "var getSettings = function() {"
  "  var request = new XMLHttpRequest();"
  "  request.onreadystatechange = function(response) {"
  "    if(request.readyState === 4) {"
  "      if(request.status === 200) {"
  "        var jsonOptions = JSON.parse(request.responseText);"
  "        for(var key in jsonOptions) {"
  "          var el = document.getElementsByName(key);"
  "          if(el.length > 0) {"
  "            if(Array(\"text\", \"number\", \"password\").indexOf(el[0].type) > -1) {"
  "              el[0].value = jsonOptions[key];"
  "            };"
  "            if(el[0].type == \"checkbox\" && jsonOptions[key] == 1) {"
  "              el[0].checked = true;"
  "              if(key.indexOf(\"1wire\") > -1) {"
  "                ShowHideDallasTable(el[0]);"
  "              };"
  "              if(key.indexOf(\"s0\") > -1) {"
  "                ShowHideS0Table(el[0]);"
  "              };"
  "            };"
  "            if(el[0].type == \"radio\") {"
  "              for(var x = 0; x < el.length; x++) {"
  "                if(el[x].value == jsonOptions[key]) {"
  "                  el[x].checked = true;"
  "                };"
  "              };"
  "            };"
  "            if(el[0].type.indexOf(\"select\") > -1) {"
  "              var children = el[0].childNodes;"
  "              for(var x = 0; x < children.length; x++) {"
  "                if(children[x].value == jsonOptions[key]) {"
  "                  children[x].selected = true;"
  "                };"
  "              };"
  "            };"
  "          };"
  "        };"
  "        document.getElementById(\"loading_settings\").style.display = \"none\";"
  "        document.getElementById(\"settings_form\").style.display = \"block\";"
  "        changeMinWatt(1);"
  "        changeMinWatt(2);"
  "      };"
  "    };"
  "  };"
  "  request.open('GET', '/getsettings', true);"
  "  request.send();"
  "};"
  "getSettings();"
  "</script>";

static const char showFirmwarePage[] PROGMEM =
  "<div class=\"w3-sidebar w3-bar-block w3-card w3-animate-left\" style=\"display:none\" id=\"leftMenu\">"
  "<a href=\"/\" class=\"w3-bar-item w3-button\">Home</a>"
  "<a href=\"/reboot\" class=\"w3-bar-item w3-button\">Reboot</a>"
  "<a href=\"/settings\" class=\"w3-bar-item w3-button\">Settings</a>"
  "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>"
  "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>"
  "</div>"
  "<div class=\"w3-container w3-center\">"
  "   <form method='POST' action='' enctype='multipart/form-data'>"
  "       Firmware:<br>"
  "       <input type='file' accept='.bin,.bin.gz' name='firmware'><br><br>"
  "       <input type='submit' value='Update Firmware'>"
  "   </form>"
  "</div>";

static const char firmwareSuccessResponse[] PROGMEM =
  "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update success! Rebooting...";

static const char firmwareFailResponse[] PROGMEM =
  "<META http-equiv=\"refresh\" content=\"15;URL=/firmware\">Update failed! Please try again...";

// https://github.com/nayarsystems/posix_tz_db
struct tzStruct {
  char name[32];
  char value[46];
};
const tzStruct tzdata[] PROGMEM = {
  { "ETC/GMT", "GMT0" },
  { "Africa/Abidjan", "GMT0" },
	{ "Africa/Accra", "GMT0" },
	{ "Africa/Addis_Ababa", "EAT-3" },
	{ "Africa/Algiers", "CET-1" },
	{ "Africa/Asmara", "EAT-3" },
	{ "Africa/Bamako", "GMT0" },
	{ "Africa/Bangui", "WAT-1" },
	{ "Africa/Banjul", "GMT0" },
	{ "Africa/Bissau", "GMT0" },
	{ "Africa/Blantyre", "CAT-2" },
	{ "Africa/Brazzaville", "WAT-1" },
	{ "Africa/Bujumbura", "CAT-2" },
	{ "Africa/Cairo", "EET-2" },
	{ "Africa/Casablanca", "<+01>-1" },
	{ "Africa/Ceuta", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Africa/Conakry", "GMT0" },
	{ "Africa/Dakar", "GMT0" },
	{ "Africa/Dar_es_Salaam", "EAT-3" },
	{ "Africa/Djibouti", "EAT-3" },
	{ "Africa/Douala", "WAT-1" },
	{ "Africa/El_Aaiun", "<+01>-1" },
	{ "Africa/Freetown", "GMT0" },
	{ "Africa/Gaborone", "CAT-2" },
	{ "Africa/Harare", "CAT-2" },
	{ "Africa/Johannesburg", "SAST-2" },
	{ "Africa/Juba", "CAT-2" },
	{ "Africa/Kampala", "EAT-3" },
	{ "Africa/Khartoum", "CAT-2" },
	{ "Africa/Kigali", "CAT-2" },
	{ "Africa/Kinshasa", "WAT-1" },
	{ "Africa/Lagos", "WAT-1" },
	{ "Africa/Libreville", "WAT-1" },
	{ "Africa/Lome", "GMT0" },
	{ "Africa/Luanda", "WAT-1" },
	{ "Africa/Lubumbashi", "CAT-2" },
	{ "Africa/Lusaka", "CAT-2" },
	{ "Africa/Malabo", "WAT-1" },
	{ "Africa/Maputo", "CAT-2" },
	{ "Africa/Maseru", "SAST-2" },
	{ "Africa/Mbabane", "SAST-2" },
	{ "Africa/Mogadishu", "EAT-3" },
	{ "Africa/Monrovia", "GMT0" },
	{ "Africa/Nairobi", "EAT-3" },
	{ "Africa/Ndjamena", "WAT-1" },
	{ "Africa/Niamey", "WAT-1" },
	{ "Africa/Nouakchott", "GMT0" },
	{ "Africa/Ouagadougou", "GMT0" },
	{ "Africa/Porto-Novo", "WAT-1" },
	{ "Africa/Sao_Tome", "GMT0" },
	{ "Africa/Tripoli", "EET-2" },
	{ "Africa/Tunis", "CET-1" },
	{ "Africa/Windhoek", "CAT-2" },
	{ "America/Adak", "HST10HDT,M3.2.0,M11.1.0" },
	{ "America/Anchorage", "AKST9AKDT,M3.2.0,M11.1.0" },
	{ "America/Anguilla", "AST4" },
	{ "America/Antigua", "AST4" },
	{ "America/Araguaina", "<-03>3" },
	{ "America/Argentina/Buenos_Aires", "<-03>3" },
	{ "America/Argentina/Catamarca", "<-03>3" },
	{ "America/Argentina/Cordoba", "<-03>3" },
	{ "America/Argentina/Jujuy", "<-03>3" },
	{ "America/Argentina/La_Rioja", "<-03>3" },
	{ "America/Argentina/Mendoza", "<-03>3" },
	{ "America/Argentina/Rio_Gallegos", "<-03>3" },
	{ "America/Argentina/Salta", "<-03>3" },
	{ "America/Argentina/San_Juan", "<-03>3" },
	{ "America/Argentina/San_Luis", "<-03>3" },
	{ "America/Argentina/Tucuman", "<-03>3" },
	{ "America/Argentina/Ushuaia", "<-03>3" },
	{ "America/Aruba", "AST4" },
	{ "America/Asuncion", "<-04>4<-03>,M10.1.0/0,M3.4.0/0" },
	{ "America/Atikokan", "EST5" },
	{ "America/Bahia", "<-03>3" },
	{ "America/Bahia_Banderas", "CST6CDT,M4.1.0,M10.5.0" },
	{ "America/Barbados", "AST4" },
	{ "America/Belem", "<-03>3" },
	{ "America/Belize", "CST6" },
	{ "America/Blanc-Sablon", "AST4" },
	{ "America/Boa_Vista", "<-04>4" },
	{ "America/Bogota", "<-05>5" },
	{ "America/Boise", "MST7MDT,M3.2.0,M11.1.0" },
	{ "America/Cambridge_Bay", "MST7MDT,M3.2.0,M11.1.0" },
	{ "America/Campo_Grande", "<-04>4" },
	{ "America/Cancun", "EST5" },
	{ "America/Caracas", "<-04>4" },
	{ "America/Cayenne", "<-03>3" },
	{ "America/Cayman", "EST5" },
	{ "America/Chicago", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Chihuahua", "MST7MDT,M4.1.0,M10.5.0" },
	{ "America/Costa_Rica", "CST6" },
	{ "America/Creston", "MST7" },
	{ "America/Cuiaba", "<-04>4" },
	{ "America/Curacao", "AST4" },
	{ "America/Danmarkshavn", "GMT0" },
	{ "America/Dawson", "MST7" },
	{ "America/Dawson_Creek", "MST7" },
	{ "America/Denver", "MST7MDT,M3.2.0,M11.1.0" },
	{ "America/Detroit", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Dominica", "AST4" },
	{ "America/Edmonton", "MST7MDT,M3.2.0,M11.1.0" },
	{ "America/Eirunepe", "<-05>5" },
	{ "America/El_Salvador", "CST6" },
	{ "America/Fortaleza", "<-03>3" },
	{ "America/Fort_Nelson", "MST7" },
	{ "America/Glace_Bay", "AST4ADT,M3.2.0,M11.1.0" },
	{ "America/Godthab", "<-03>3<-02>,M3.5.0/-2,M10.5.0/-1" },
	{ "America/Goose_Bay", "AST4ADT,M3.2.0,M11.1.0" },
	{ "America/Grand_Turk", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Grenada", "AST4" },
	{ "America/Guadeloupe", "AST4" },
	{ "America/Guatemala", "CST6" },
	{ "America/Guayaquil", "<-05>5" },
	{ "America/Guyana", "<-04>4" },
	{ "America/Halifax", "AST4ADT,M3.2.0,M11.1.0" },
	{ "America/Havana", "CST5CDT,M3.2.0/0,M11.1.0/1" },
	{ "America/Hermosillo", "MST7" },
	{ "America/Indiana/Indianapolis", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Indiana/Knox", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Indiana/Marengo", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Indiana/Petersburg", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Indiana/Tell_City", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Indiana/Vevay", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Indiana/Vincennes", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Indiana/Winamac", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Inuvik", "MST7MDT,M3.2.0,M11.1.0" },
	{ "America/Iqaluit", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Jamaica", "EST5" },
	{ "America/Juneau", "AKST9AKDT,M3.2.0,M11.1.0" },
	{ "America/Kentucky/Louisville", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Kentucky/Monticello", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Kralendijk", "AST4" },
	{ "America/La_Paz", "<-04>4" },
	{ "America/Lima", "<-05>5" },
	{ "America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0" },
	{ "America/Lower_Princes", "AST4" },
	{ "America/Maceio", "<-03>3" },
	{ "America/Managua", "CST6" },
	{ "America/Manaus", "<-04>4" },
	{ "America/Marigot", "AST4" },
	{ "America/Martinique", "AST4" },
	{ "America/Matamoros", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Mazatlan", "MST7MDT,M4.1.0,M10.5.0" },
	{ "America/Menominee", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Merida", "CST6CDT,M4.1.0,M10.5.0" },
	{ "America/Metlakatla", "AKST9AKDT,M3.2.0,M11.1.0" },
	{ "America/Mexico_City", "CST6CDT,M4.1.0,M10.5.0" },
	{ "America/Miquelon", "<-03>3<-02>,M3.2.0,M11.1.0" },
	{ "America/Moncton", "AST4ADT,M3.2.0,M11.1.0" },
	{ "America/Monterrey", "CST6CDT,M4.1.0,M10.5.0" },
	{ "America/Montevideo", "<-03>3" },
	{ "America/Montreal", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Montserrat", "AST4" },
	{ "America/Nassau", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/New_York", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Nipigon", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Nome", "AKST9AKDT,M3.2.0,M11.1.0" },
	{ "America/Noronha", "<-02>2" },
	{ "America/North_Dakota/Beulah", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/North_Dakota/Center", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/North_Dakota/New_Salem", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Nuuk", "<-03>3<-02>,M3.5.0/-2,M10.5.0/-1" },
	{ "America/Ojinaga", "MST7MDT,M3.2.0,M11.1.0" },
	{ "America/Panama", "EST5" },
	{ "America/Pangnirtung", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Paramaribo", "<-03>3" },
	{ "America/Phoenix", "MST7" },
	{ "America/Port-au-Prince", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Port_of_Spain", "AST4" },
	{ "America/Porto_Velho", "<-04>4" },
	{ "America/Puerto_Rico", "AST4" },
	{ "America/Punta_Arenas", "<-03>3" },
	{ "America/Rainy_River", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Rankin_Inlet", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Recife", "<-03>3" },
	{ "America/Regina", "CST6" },
	{ "America/Resolute", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Rio_Branco", "<-05>5" },
	{ "America/Santarem", "<-03>3" },
	{ "America/Santiago", "<-04>4<-03>,M9.1.6/24,M4.1.6/24" },
	{ "America/Santo_Domingo", "AST4" },
	{ "America/Sao_Paulo", "<-03>3" },
	{ "America/Scoresbysund", "<-01>1<+00>,M3.5.0/0,M10.5.0/1" },
	{ "America/Sitka", "AKST9AKDT,M3.2.0,M11.1.0" },
	{ "America/St_Barthelemy", "AST4" },
	{ "America/St_Johns", "NST3:30NDT,M3.2.0,M11.1.0" },
	{ "America/St_Kitts", "AST4" },
	{ "America/St_Lucia", "AST4" },
	{ "America/St_Thomas", "AST4" },
	{ "America/St_Vincent", "AST4" },
	{ "America/Swift_Current", "CST6" },
	{ "America/Tegucigalpa", "CST6" },
	{ "America/Thule", "AST4ADT,M3.2.0,M11.1.0" },
	{ "America/Thunder_Bay", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Tijuana", "PST8PDT,M3.2.0,M11.1.0" },
	{ "America/Toronto", "EST5EDT,M3.2.0,M11.1.0" },
	{ "America/Tortola", "AST4" },
	{ "America/Vancouver", "PST8PDT,M3.2.0,M11.1.0" },
	{ "America/Whitehorse", "MST7" },
	{ "America/Winnipeg", "CST6CDT,M3.2.0,M11.1.0" },
	{ "America/Yakutat", "AKST9AKDT,M3.2.0,M11.1.0" },
	{ "America/Yellowknife", "MST7MDT,M3.2.0,M11.1.0" },
	{ "Antarctica/Casey", "<+11>-11" },
	{ "Antarctica/Davis", "<+07>-7" },
	{ "Antarctica/DumontDUrville", "<+10>-10" },
	{ "Antarctica/Macquarie", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
	{ "Antarctica/Mawson", "<+05>-5" },
	{ "Antarctica/McMurdo", "NZST-12NZDT,M9.5.0,M4.1.0/3" },
	{ "Antarctica/Palmer", "<-03>3" },
	{ "Antarctica/Rothera", "<-03>3" },
	{ "Antarctica/Syowa", "<+03>-3" },
	{ "Antarctica/Troll", "<+00>0<+02>-2,M3.5.0/1,M10.5.0/3" },
	{ "Antarctica/Vostok", "<+06>-6" },
	{ "Arctic/Longyearbyen", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Asia/Aden", "<+03>-3" },
	{ "Asia/Almaty", "<+06>-6" },
	{ "Asia/Amman", "EET-2EEST,M2.5.4/24,M10.5.5/1" },
	{ "Asia/Anadyr", "<+12>-12" },
	{ "Asia/Aqtau", "<+05>-5" },
	{ "Asia/Aqtobe", "<+05>-5" },
	{ "Asia/Ashgabat", "<+05>-5" },
	{ "Asia/Atyrau", "<+05>-5" },
	{ "Asia/Baghdad", "<+03>-3" },
	{ "Asia/Bahrain", "<+03>-3" },
	{ "Asia/Baku", "<+04>-4" },
	{ "Asia/Bangkok", "<+07>-7" },
	{ "Asia/Barnaul", "<+07>-7" },
	{ "Asia/Beirut", "EET-2EEST,M3.5.0/0,M10.5.0/0" },
	{ "Asia/Bishkek", "<+06>-6" },
	{ "Asia/Brunei", "<+08>-8" },
	{ "Asia/Chita", "<+09>-9" },
	{ "Asia/Choibalsan", "<+08>-8" },
	{ "Asia/Colombo", "<+0530>-5:30" },
	{ "Asia/Damascus", "EET-2EEST,M3.5.5/0,M10.5.5/0" },
	{ "Asia/Dhaka", "<+06>-6" },
	{ "Asia/Dili", "<+09>-9" },
	{ "Asia/Dubai", "<+04>-4" },
	{ "Asia/Dushanbe", "<+05>-5" },
	{ "Asia/Famagusta", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Asia/Gaza", "EET-2EEST,M3.4.4/48,M10.5.5/1" },
	{ "Asia/Hebron", "EET-2EEST,M3.4.4/48,M10.5.5/1" },
	{ "Asia/Ho_Chi_Minh", "<+07>-7" },
	{ "Asia/Hong_Kong", "HKT-8" },
	{ "Asia/Hovd", "<+07>-7" },
	{ "Asia/Irkutsk", "<+08>-8" },
	{ "Asia/Jakarta", "WIB-7" },
	{ "Asia/Jayapura", "WIT-9" },
	{ "Asia/Jerusalem", "IST-2IDT,M3.4.4/26,M10.5.0" },
	{ "Asia/Kabul", "<+0430>-4:30" },
	{ "Asia/Kamchatka", "<+12>-12" },
	{ "Asia/Karachi", "PKT-5" },
	{ "Asia/Kathmandu", "<+0545>-5:45" },
	{ "Asia/Khandyga", "<+09>-9" },
	{ "Asia/Kolkata", "IST-5:30" },
	{ "Asia/Krasnoyarsk", "<+07>-7" },
	{ "Asia/Kuala_Lumpur", "<+08>-8" },
	{ "Asia/Kuching", "<+08>-8" },
	{ "Asia/Kuwait", "<+03>-3" },
	{ "Asia/Macau", "CST-8" },
	{ "Asia/Magadan", "<+11>-11" },
	{ "Asia/Makassar", "WITA-8" },
	{ "Asia/Manila", "PST-8" },
	{ "Asia/Muscat", "<+04>-4" },
	{ "Asia/Nicosia", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Asia/Novokuznetsk", "<+07>-7" },
	{ "Asia/Novosibirsk", "<+07>-7" },
	{ "Asia/Omsk", "<+06>-6" },
	{ "Asia/Oral", "<+05>-5" },
	{ "Asia/Phnom_Penh", "<+07>-7" },
	{ "Asia/Pontianak", "WIB-7" },
	{ "Asia/Pyongyang", "KST-9" },
	{ "Asia/Qatar", "<+03>-3" },
	{ "Asia/Qyzylorda", "<+05>-5" },
	{ "Asia/Riyadh", "<+03>-3" },
	{ "Asia/Sakhalin", "<+11>-11" },
	{ "Asia/Samarkand", "<+05>-5" },
	{ "Asia/Seoul", "KST-9" },
	{ "Asia/Shanghai", "CST-8" },
	{ "Asia/Singapore", "<+08>-8" },
	{ "Asia/Srednekolymsk", "<+11>-11" },
	{ "Asia/Taipei", "CST-8" },
	{ "Asia/Tashkent", "<+05>-5" },
	{ "Asia/Tbilisi", "<+04>-4" },
	{ "Asia/Tehran", "<+0330>-3:30<+0430>,J79/24,J263/24" },
	{ "Asia/Thimphu", "<+06>-6" },
	{ "Asia/Tokyo", "JST-9" },
	{ "Asia/Tomsk", "<+07>-7" },
	{ "Asia/Ulaanbaatar", "<+08>-8" },
	{ "Asia/Urumqi", "<+06>-6" },
	{ "Asia/Ust-Nera", "<+10>-10" },
	{ "Asia/Vientiane", "<+07>-7" },
	{ "Asia/Vladivostok", "<+10>-10" },
	{ "Asia/Yakutsk", "<+09>-9" },
	{ "Asia/Yangon", "<+0630>-6:30" },
	{ "Asia/Yekaterinburg", "<+05>-5" },
	{ "Asia/Yerevan", "<+04>-4" },
	{ "Atlantic/Azores", "<-01>1<+00>,M3.5.0/0,M10.5.0/1" },
	{ "Atlantic/Bermuda", "AST4ADT,M3.2.0,M11.1.0" },
	{ "Atlantic/Canary", "WET0WEST,M3.5.0/1,M10.5.0" },
	{ "Atlantic/Cape_Verde", "<-01>1" },
	{ "Atlantic/Faroe", "WET0WEST,M3.5.0/1,M10.5.0" },
	{ "Atlantic/Madeira", "WET0WEST,M3.5.0/1,M10.5.0" },
	{ "Atlantic/Reykjavik", "GMT0" },
	{ "Atlantic/South_Georgia", "<-02>2" },
	{ "Atlantic/Stanley", "<-03>3" },
	{ "Atlantic/St_Helena", "GMT0" },
	{ "Australia/Adelaide", "ACST-9:30ACDT,M10.1.0,M4.1.0/3" },
	{ "Australia/Brisbane", "AEST-10" },
	{ "Australia/Broken_Hill", "ACST-9:30ACDT,M10.1.0,M4.1.0/3" },
	{ "Australia/Currie", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
	{ "Australia/Darwin", "ACST-9:30" },
	{ "Australia/Eucla", "<+0845>-8:45" },
	{ "Australia/Hobart", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
	{ "Australia/Lindeman", "AEST-10" },
	{ "Australia/Lord_Howe", "<+1030>-10:30<+11>-11,M10.1.0,M4.1.0" },
	{ "Australia/Melbourne", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
	{ "Australia/Perth", "AWST-8" },
	{ "Australia/Sydney", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
	{ "Europe/Amsterdam", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Andorra", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Astrakhan", "<+04>-4" },
	{ "Europe/Athens", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Belgrade", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Berlin", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Bratislava", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Brussels", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Bucharest", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Budapest", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Busingen", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Chisinau", "EET-2EEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Copenhagen", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Dublin", "IST-1GMT0,M10.5.0,M3.5.0/1" },
	{ "Europe/Gibraltar", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Guernsey", "GMT0BST,M3.5.0/1,M10.5.0" },
	{ "Europe/Helsinki", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Isle_of_Man", "GMT0BST,M3.5.0/1,M10.5.0" },
	{ "Europe/Istanbul", "<+03>-3" },
	{ "Europe/Jersey", "GMT0BST,M3.5.0/1,M10.5.0" },
	{ "Europe/Kaliningrad", "EET-2" },
	{ "Europe/Kiev", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Kirov", "<+03>-3" },
	{ "Europe/Lisbon", "WET0WEST,M3.5.0/1,M10.5.0" },
	{ "Europe/Ljubljana", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/London", "GMT0BST,M3.5.0/1,M10.5.0" },
	{ "Europe/Luxembourg", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Madrid", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Malta", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Mariehamn", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Minsk", "<+03>-3" },
	{ "Europe/Monaco", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Moscow", "MSK-3" },
	{ "Europe/Oslo", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Paris", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Podgorica", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Prague", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Riga", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Rome", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Samara", "<+04>-4" },
	{ "Europe/San_Marino", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Sarajevo", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Saratov", "<+04>-4" },
	{ "Europe/Simferopol", "MSK-3" },
	{ "Europe/Skopje", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Sofia", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Stockholm", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Tallinn", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Tirane", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Ulyanovsk", "<+04>-4" },
	{ "Europe/Uzhgorod", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Vaduz", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Vatican", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Vienna", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Vilnius", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Volgograd", "<+03>-3" },
	{ "Europe/Warsaw", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Zagreb", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Europe/Zaporozhye", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
	{ "Europe/Zurich", "CET-1CEST,M3.5.0,M10.5.0/3" },
	{ "Indian/Antananarivo", "EAT-3" },
	{ "Indian/Chagos", "<+06>-6" },
	{ "Indian/Christmas", "<+07>-7" },
	{ "Indian/Cocos", "<+0630>-6:30" },
	{ "Indian/Comoro", "EAT-3" },
	{ "Indian/Kerguelen", "<+05>-5" },
	{ "Indian/Mahe", "<+04>-4" },
	{ "Indian/Maldives", "<+05>-5" },
	{ "Indian/Mauritius", "<+04>-4" },
	{ "Indian/Mayotte", "EAT-3" },
	{ "Indian/Reunion", "<+04>-4" },
	{ "Pacific/Apia", "<+13>-13" },
	{ "Pacific/Auckland", "NZST-12NZDT,M9.5.0,M4.1.0/3" },
	{ "Pacific/Bougainville", "<+11>-11" },
	{ "Pacific/Chatham", "<+1245>-12:45<+1345>,M9.5.0/2:45,M4.1.0/3:45" },
	{ "Pacific/Chuuk", "<+10>-10" },
	{ "Pacific/Easter", "<-06>6<-05>,M9.1.6/22,M4.1.6/22" },
	{ "Pacific/Efate", "<+11>-11" },
	{ "Pacific/Enderbury", "<+13>-13" },
	{ "Pacific/Fakaofo", "<+13>-13" },
	{ "Pacific/Fiji", "<+12>-12<+13>,M11.2.0,M1.2.3/99" },
	{ "Pacific/Funafuti", "<+12>-12" },
	{ "Pacific/Galapagos", "<-06>6" },
	{ "Pacific/Gambier", "<-09>9" },
	{ "Pacific/Guadalcanal", "<+11>-11" },
	{ "Pacific/Guam", "ChST-10" },
	{ "Pacific/Honolulu", "HST10" },
	{ "Pacific/Kiritimati", "<+14>-14" },
	{ "Pacific/Kosrae", "<+11>-11" },
	{ "Pacific/Kwajalein", "<+12>-12" },
	{ "Pacific/Majuro", "<+12>-12" },
	{ "Pacific/Marquesas", "<-0930>9:30" },
	{ "Pacific/Midway", "SST11" },
	{ "Pacific/Nauru", "<+12>-12" },
	{ "Pacific/Niue", "<-11>11" },
	{ "Pacific/Norfolk", "<+11>-11<+12>,M10.1.0,M4.1.0/3" },
	{ "Pacific/Noumea", "<+11>-11" },
	{ "Pacific/Pago_Pago", "SST11" },
	{ "Pacific/Palau", "<+09>-9" },
	{ "Pacific/Pitcairn", "<-08>8" },
	{ "Pacific/Pohnpei", "<+11>-11" },
	{ "Pacific/Port_Moresby", "<+10>-10" },
	{ "Pacific/Rarotonga", "<-10>10" },
	{ "Pacific/Saipan", "ChST-10" },
	{ "Pacific/Tahiti", "<-10>10" },
	{ "Pacific/Tarawa", "<+12>-12" },
	{ "Pacific/Tongatapu", "<+13>-13" },
	{ "Pacific/Wake", "<+12>-12" },
	{ "Pacific/Wallis", "<+12>-12" },
	{ "Etc/GMT-0", "GMT0" },
	{ "Etc/GMT-1", "<+01>-1" },
	{ "Etc/GMT-2", "<+02>-2" },
	{ "Etc/GMT-3", "<+03>-3" },
	{ "Etc/GMT-4", "<+04>-4" },
	{ "Etc/GMT-5", "<+05>-5" },
	{ "Etc/GMT-6", "<+06>-6" },
	{ "Etc/GMT-7", "<+07>-7" },
	{ "Etc/GMT-8", "<+08>-8" },
	{ "Etc/GMT-9", "<+09>-9" },
	{ "Etc/GMT-10", "<+10>-10" },
	{ "Etc/GMT-11", "<+11>-11" },
	{ "Etc/GMT-12", "<+12>-12" },
	{ "Etc/GMT-13", "<+13>-13" },
	{ "Etc/GMT-14", "<+14>-14" },
	{ "Etc/GMT0", "GMT0" },
	{ "Etc/GMT+0", "GMT0" },
	{ "Etc/GMT+1", "<-01>1" },
	{ "Etc/GMT+2", "<-02>2" },
	{ "Etc/GMT+3", "<-03>3" },
	{ "Etc/GMT+4", "<-04>4" },
	{ "Etc/GMT+5", "<-05>5" },
	{ "Etc/GMT+6", "<-06>6" },
	{ "Etc/GMT+7", "<-07>7" },
	{ "Etc/GMT+8", "<-08>8" },
	{ "Etc/GMT+9", "<-09>9" },
	{ "Etc/GMT+10", "<-10>10" },
	{ "Etc/GMT+11", "<-11>11" },
	{ "Etc/GMT+12", "<-12>12" },
	{ "Etc/UCT", "UTC0" },
	{ "Etc/UTC", "UTC0" },
	{ "Etc/Greenwich", "GMT0" },
	{ "Etc/Universal", "UTC0" },
	{ "Etc/Zulu", "UTC0" },
};
