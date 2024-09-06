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
  "      oWebsocket = new MozWebSocket(\"ws://\" + location.host + \":80\");"
  "    } else if(typeof WebSocket != \"undefined\") {"
  "      /* The characters after the trailing slash are needed for a wierd IE 10 bug */"
  "      oWebsocket = new WebSocket(\"ws://\" + location.host + \":80/ws\");"
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
  "        if (evt.data.startsWith('{')) {"
  "          const jsonObject = JSON.parse(evt.data);"
  "          if (jsonObject.hasOwnProperty('logMsg')) {"
  "            let obj = document.getElementById(\"cli\");"
  "            let chk = document.getElementById(\"autoscroll\");"
  "            obj.value += jsonObject['logMsg'] + \"\\n\";"
  "            if(chk.checked) {"
  "              obj.scrollTop = obj.scrollHeight;"
  "            }"
  "          } else if (jsonObject.hasOwnProperty('data')) {"
  "          }"
  "        } else {"
  "          let obj = document.getElementById(\"cli\");"
  "          let chk = document.getElementById(\"autoscroll\");"
  "          obj.value += evt.data + \"\\n\";"
  "          if(chk.checked) {"
  "            obj.scrollTop = obj.scrollHeight;"
  "          }"
  "        }"
  "      }"
  "    }"
  "  }"
  "</script>";

static const char refreshJS[] PROGMEM =
  "<script>"
  " let timeout;"
  " let isEditing = false;"
  " document.body.onload=function() {"
  "    setInterval(refreshTable,30000);"
  "    openTable('Heatpump');"
  "    document.getElementById(\"cli\").value = \"\";"
  "    startWebsockets();"
  "    refreshTable();"
  " };"
  " var dallasAliasEdit = function() {"
  "   isEditing = false;"
  "   var address = this.getAttribute(\"data-address\");"
  "   var alias = this.innerText.substring(0,30);"
  "   var xhr = new XMLHttpRequest();"
  "   var url = \"/dallasalias?\"+address+\"=\"+alias;"
  "   xhr.open('GET', url, true);"
  "   xhr.send();"
  " };"
  " async function refreshTable(tableName){"
  "   try {"
  "     if (isEditing) {"
  "       return;"
  "     }"
  "     const response = await fetch('/json');"  
  "     const jsonData = await response.json();"  
  "     if (jsonData?.heatpump && Array.isArray(jsonData.heatpump)) {"  
  "       const tableBody = document.getElementById('heishavalues');"  
  "       tableBody.innerHTML = '';"  
  "       jsonData.heatpump.forEach(item => {"
  "         const row = document.createElement('tr');"
  "         const topic = item['Topic'];"
  "         for (const key in item) {"
  "           if (Object.hasOwn(item,key)) {"
  "             const cell = document.createElement('td');"
  "             cell.id = `${topic}-${key}`;"
  "             cell.textContent = item[key];"
  "             row.appendChild(cell);"
  "           }"
  "         }"
  "         tableBody.appendChild(row);"
  "       });"  
  "     }"
  "     if (jsonData?.['heatpump extra'] && Array.isArray(jsonData['heatpump extra'])) {"  
  "       const tableBody = document.getElementById('heishavalues');"  
  "       jsonData['heatpump extra'].forEach(item => {"
  "         const row = document.createElement('tr');"
  "         const topic = item['Topic'];"  
  "         for (const key in item) {"
  "           if (Object.hasOwn(item,key)) {"
  "             const cell = document.createElement('td');"
  "             cell.id = `${topic}-${key}`;"
  "             cell.textContent = item[key];"
  "             row.appendChild(cell);"
  "           }"
  "         }"
  "         tableBody.appendChild(row);"
  "       });"  
  "     }"
  "     if (jsonData?.['heatpump optional'] && Array.isArray(jsonData['heatpump optional'])) {"  
  "       const tableBody = document.getElementById('heishavalues');"  
  "       jsonData['heatpump optional'].forEach(item => {"
  "         const row = document.createElement('tr');"
  "         const topic = item['Topic'];"  
  "         for (const key in item) {"
  "           if (Object.hasOwn(item,key)) {"
  "             const cell = document.createElement('td');"
  "             cell.id = `${topic}-${key}`;"
  "             cell.textContent = item[key];"
  "             row.appendChild(cell);"
  "           }"
  "         }"
  "         tableBody.appendChild(row);"
  "       });"  
  "     }"
  "     if (jsonData?.['1wire'] && Array.isArray(jsonData['1wire'])) {"  
  "       const tableBody = document.getElementById('dallasvalues');"  
  "       tableBody.innerHTML = '';"  
  "       jsonData['1wire'].forEach(item => {"  
  "         const row = document.createElement('tr');"
  "         const sensorID = item['Sensor'];"  
  "         for (const key in item) {"
  "           if (Object.hasOwn(item,key)) {"
  "             const cell = document.createElement('td');"
  "             cell.id = `SensorID-${sensorID}-${key}`;"  
  "             if (key === 'Alias') {"
  "               const div = document.createElement('div');"
  "               div.textContent = item[key];"
  "               div.classList.add('w3-border','w3-border-light-grey','w3-hover-border-black');"
  "               div.contentEditable = 'true';"
  "               div.setAttribute('data-address', item.Sensor);"
  "               div.addEventListener('focus', () => {isEditing = true;});"
  "               div.addEventListener('blur',dallasAliasEdit);"
  "               cell.appendChild(div);"
  "             } else {"
  "               cell.textContent = item[key];"
  "             }"
  "             row.appendChild(cell);"
  "           }"
  "         }"
  "         tableBody.appendChild(row);"
  "       });" 
  "     }"
  "     if (jsonData?.s0 && Array.isArray(jsonData.s0)) {"  
  "       const tableBody = document.getElementById('s0values');"  
  "       tableBody.innerHTML = '';"  
  "       jsonData.s0.forEach(item => {"  
  "         const row = document.createElement('tr');"  
  "         const s0Port = item['S0 port'];"
  "         for (const key in item) {"
  "           if (Object.hasOwn(item,key)) {"
  "             const cell = document.createElement('td');"
  "             cell.textContent = item[key];"
  "             cell.id = `s0port-${s0Port}-${key}`;"  
  "             row.appendChild(cell);"
  "           }"
  "         }"
  "         tableBody.appendChild(row);"
  "       });" 
  "     }"
  "     if (jsonData?.opentherm && typeof jsonData.opentherm === 'object') {"  
  "       const tableBody = document.getElementById('openthermvalues');"  
  "       tableBody.innerHTML = '';"  
  "       for (const [key, { type, value }] of Object.entries(jsonData.opentherm)) {"  
  "         const row = document.createElement('tr');"  
  "         const nameCell = document.createElement('td');"
  "         const typeCell = document.createElement('td');"
  "         const valueCell = document.createElement('td');"
  "         nameCell.id = key;"
  "         typeCell.id = `${key}-type`;"
  "         valueCell.id = `${key}-value`;"
  "         nameCell.textContent = key;"
  "         typeCell.textContent = type;"
  "         valueCell.textContent = value;"
  "         row.appendChild(nameCell);"
  "         row.appendChild(typeCell);"
  "         row.appendChild(valueCell);"
  "         tableBody.appendChild(row);"
  "       };"  
  "     }"
  "   } catch (error) {"
  "   }"
  "}"
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
  "    function ShowHideListenOnlyTable(listenonlyEnabled) {"
  "        var listenonlysettings = document.getElementById(\"listenonlysettings\");"
  "        listenonlysettings.style.display = listenonlyEnabled.checked ? \"none\" : \"none\";"
  "    }"
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
  "<a href=\"/rules\" class=\"w3-bar-item w3-button\">Rules</a>"
  "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>"
  "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>"
  "<hr><div class=\"w3-text-grey\">Version: ";

static const char webBodyRoot2[] PROGMEM =
  "<br><a href=\"https://github.com/Egyras/HeishaMon\">Heishamon software</a></div><hr></div>"
  "<div class=\"w3-bar w3-red\">"
  "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('Heatpump')\">Heatpump</button>";

static const char webBodyRootDallasTab[] PROGMEM = "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('Dallas')\">Dallas 1-wire</button>";
static const char webBodyRootS0Tab[] PROGMEM = "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('S0')\">S0 kWh meters</button>";
static const char webBodyRootOpenthermTab[] PROGMEM = "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('Opentherm')\">Opentherm</button>";
static const char webBodyRootConsoleTab[] PROGMEM = "<button class=\"w3-bar-item w3-button\" onclick=\"openTable('Console')\">Console</button>";

static const char webBodyEndDiv[] PROGMEM = "</div>";

static const char webBodyRootStatusWifi[] PROGMEM =   "<div class=\"w3-container w3-left\"><br>Wifi signal: ";
#ifdef ESP8266
static const char webBodyRootStatusMemory[] PROGMEM =   "%<br>Memory free: ";
#else
static const char webBodyRootStatusEthernet[] PROGMEM =   "%<br>Ethernet status: ";
static const char webBodyRootStatusMemory[] PROGMEM =   "<br>Memory free: ";
#endif
static const char webBodyRootStatusReceived[] PROGMEM =  "%<br>Correct received data: ";
static const char webBodyRootStatusReconnects[] PROGMEM =  "%<br>MQTT reconnects: ";
static const char webBodyRootStatusUptime[] PROGMEM =   "<br>Uptime: ";
static const char webBodyRootStatusListenOnly[] PROGMEM =   "<br><b>Listen only mode active</b>";

static const char webBodyRootHeatpumpValues[] PROGMEM =
  "<div id=\"Heatpump\" class=\"w3-container w3-center heishatable\">"
  "<h2>Current heatpump values</h2>"
  "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Topic</th><th>Name</th><th>Value</th><th>Description</th></tr></thead><tbody id=\"heishavalues\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";

static const char webBodyRootDallasValues[] PROGMEM =
  "<div id=\"Dallas\" class=\"w3-container w3-center heishatable\" style=\"display:none\">"
  "<h2>Current Dallas 1-wire values</h2>"
  "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Sensor</th><th>Temperature</th><th>Alias</th></tr></thead><tbody id=\"dallasvalues\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";

static const char webBodyRootS0Values[] PROGMEM =
  "<div id=\"S0\" class=\"w3-container w3-center heishatable\" style=\"display:none\">"
  "<h2>Current S0 kWh meters values</h2>"
  "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>S0 port</th><th>Watt</th><th>Watthour</th><th>WatthourTotal</th><th>Pulse quality</th><th>Average pulse width</th></tr></thead><tbody id=\"s0values\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";

static const char webBodyRootOpenthermValues[] PROGMEM =
  "<div id=\"Opentherm\" class=\"w3-container w3-center heishatable\" style=\"display:none\">"
  "<h2>Current opentherm values</h2>"
  "<table class=\"w3-table-all\"><thead><tr class=\"w3-red\"><th>Name</th><th>Type</th><th>Value</th></tr></thead><tbody id=\"openthermvalues\"><tr><td>...Loading...</td><td></td></tr></tbody></table></div>";


static const char webBodyRootConsole[] PROGMEM =
  "<div id=\"Console\" class=\"w3-container w3-center heishatable\">"
  "<h2>Console output</h2>"
  "<textarea id=\"cli\" disabled></textarea><br /><input type=\"checkbox\" id=\"autoscroll\" checked=\"checked\">Enable autoscroll</div>";

static const char showRulesPage1[] PROGMEM =
  "<div class=\"w3-sidebar w3-bar-block w3-card w3-animate-left\" style=\"display:none\" id=\"leftMenu\">"
  "<a href=\"/\" class=\"w3-bar-item w3-button\">Home</a>"
  "<a href=\"/reboot\" class=\"w3-bar-item w3-button\">Reboot</a>"
  "<a href=\"/firmware\" class=\"w3-bar-item w3-button\">Firmware</a>"
  "<a href=\"/settings\" class=\"w3-bar-item w3-button\">Settings</a>"
  "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>"
  "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>"
  "</div>"
  "<div class=\"w3-container w3-center\">"
  "  <h2>Rules</h2>"
  "  <form accept-charset=\"UTF-8\" action=\"/saverules\" enctype=\"multipart/form-data\" method=\"POST\">"
  "    <textarea name=\"rules\" cols=\"75\" rows=\"15\">";

static const char showRulesPage2[] PROGMEM =
  "</textarea><br />"
  "    <input class=\"w3-green w3-button\" type=\"submit\" value=\"Save\">"
  "  </form>"
  "</div>";

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
  "<a href=\"/rules\" class=\"w3-bar-item w3-button\">Rules</a>"
  "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>"
  "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>"
  "</div>";

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
  "setTimeout(refreshWifiScan,500);"
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
  "          <input type=\"text\" name=\"wifi_hostname\" maxlength=\"39\" value=\"\">"
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
  "          <input type=\"password\" name=\"wifi_password\" maxlength=\"64\" value=\"\">"
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
  "          <input type=\"password\" name=\"current_ota_password\" maxlength=\"39\" value=\"\"> default password: \"heisha\""
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          New update password:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"password\" name=\"new_ota_password\" maxlength=\"39\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt topic base:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"mqtt_topic_base\" maxlength=\"127\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt server:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"mqtt_server\" maxlength=\"64\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt port:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"number\" name=\"mqtt_port\" maxlength=\"5\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt username:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"mqtt_username\" maxlength=\"64\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt password:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"password\" name=\"mqtt_password\" maxlength=\"64\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          NTP servers (comma separated):</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"ntp_servers\" maxlength=\"253\" value=\"\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Timezone:</td>"
  "        <td style=\"text-align:left\">"
  "          <select name=\"timezone\">";

#ifdef ESP8266
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
  "          Enable WiFi hotspot when not connected:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"hotspot\" value=\"enabled\">"
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
  "          Emulate optional PCB (does not work in listen only mode):</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"optionalPCB\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Enable opentherm processing (requires opentherm pcb):</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"opentherm\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "    </table>"
  "    <table style=\"width:100%\">"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Listen only (parallel CZ-TAW1):</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"listenonly\" onclick=\"ShowHideListenOnlyTable(this)\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "    </table>"
  "    <table id=\"listenonlysettings\" style=\"display: none; width:100%\">"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Listen to data over mqtt:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"listenmqtt\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt base topic to listen to:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"mqtt_topic_listen\" maxlength=\"127\" value=\"\">"
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
#else
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
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Enable WiFi hotspot when not connected:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"hotspot\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
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
  "          Debug log USB:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"logSerial1\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Emulate optional PCB (does not work in listen only mode):</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"optionalPCB\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Enable opentherm processing:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"opentherm\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Enable cztaw proxy port:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"proxy\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "    </table>"
  "    <table style=\"width:100%\">"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Listen only (parallel CZ-TAW1):</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"listenonly\" onclick=\"ShowHideListenOnlyTable(this)\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "    </table>"
  "    <table id=\"listenonlysettings\" style=\"display: none; width:100%\">"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Listen to data over mqtt:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"checkbox\" name=\"listenmqtt\" value=\"enabled\">"
  "        </td>"
  "      </tr>"
  "      <tr>"
  "        <td style=\"text-align:right; width: 50%\">"
  "          Mqtt base topic to listen to:</td>"
  "        <td style=\"text-align:left\">"
  "          <input type=\"text\" name=\"mqtt_topic_listen\" maxlength=\"127\" value=\"\">"
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
#endif
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
  "              if(key.indexOf(\"listenonly\") > -1) {"
  "                ShowHideListenOnlyTable(el[0]);"
  "              };"
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
  "<script>"
  "  function getMD5(){"
  "    var filename = document.getElementById(\"firmware\").value;"
  "    var splitstr = filename.split('-')[2];"
  "    if (splitstr) {"
  "      var md5 = splitstr.split('.')[0];"
  "      if (md5.length == 32) {"
  "        document.getElementById(\"md5\").value = md5;"
  "      }"
  "    }"
  "  }"
  "  function reloadHome() {"
  "    setTimeout(function() {window.location.href = \"/\";}, 15000);"
  "  }"
  "  function _(el) {  "
  "    return document.getElementById(el);  "
  "  }  "
  "  "
  "  function uploadFile() {  "
  "    _(\"updatebutton\").disabled = true;"
  "    _(\"status\").innerText = \"\";"
  "    var file = _(\"firmware\").files[0];  "
  "    var formdata = new FormData();  "
  "    formdata.append(\"firmware\", file);  "
  "    var md5 = document.getElementById(\"md5\").value;"
  "    formdata.append(\"md5\", md5);"
  "    var request = new XMLHttpRequest();  "
  "    request.upload.addEventListener(\"progress\", progressHandler, false);"
  "    request.onreadystatechange = function(response) {"
  "      if (request.readyState === 4) {"
  "        _(\"status\").innerText = request.responseText;"
  "        if (request.responseText.includes(\"success\")) { "
  "          reloadHome();"
  "        } else {"
  "          _(\"updatebutton\").disabled = false;"
  "        };"
  "      };"
  "    };"
  "    request.open(\"POST\", \"/firmware\");"
  "    request.send(formdata);"
  "  }  "
  "  "
  "  function progressHandler(event) {  "
  "    var percent = (event.loaded / event.total) * 100;  "
  "    _(\"progressBar\").value = Math.round(percent);  "
  "  }  "
  "</script>"
  "<div class=\"w3-sidebar w3-bar-block w3-card w3-animate-left\" style=\"display:none\" id=\"leftMenu\">"
  "<a href=\"/\" class=\"w3-bar-item w3-button\">Home</a>"
  "<a href=\"/reboot\" class=\"w3-bar-item w3-button\">Reboot</a>"
  "<a href=\"/settings\" class=\"w3-bar-item w3-button\">Settings</a>"
  "<a href=\"/togglelog\" class=\"w3-bar-item w3-button\">Toggle mqtt log</a>"
  "<a href=\"/togglehexdump\" class=\"w3-bar-item w3-button\">Toggle hexdump log</a>"
  "</div>"
  "<div class=\"w3-container w3-center\">"
  "   <form method=\"POST\" action=\"\" enctype=\"multipart/form-data\">"
  "       <h2>Firmware:</h2>"
  "       <input type=\"file\" accept=\".bin,.bin.gz\" id=\"firmware\" name=\"firmware\" onchange=\"getMD5();\"><br><br>"
  "       <label for=\"md5\">MD5 checksum:</label><input type=\"text\" id=\"md5\" name=\"md5\" value=\"\" size=\"32\" minlength=\"32\" maxlength=\"32\"><br><br><b>Warning</b><br>If you leave the MD5 checksum empty there will be no check on the uploaded firmware which could cause a bricked HeishaMon!<br>In this case but also other unforseen errors during update requires you to be able to restore the firmware using a TTL cable!<br><br>"
  "   </form>"
  "   <button id=\"updatebutton\" onclick=\"uploadFile()\">Update Firmware</button><br><progress id=\"progressBar\" value=\"0\" max=\"100\" style=\"width:300px;\"></progress><p id=\"status\"></p>"
  "</div>";

static const char firmwareSuccessResponse[] PROGMEM =
  "Update success! Rebooting. This page will refresh afterwards.";

static const char firmwareFailResponse[] PROGMEM =
  "Update failed! Please try again...";

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

static const char tzDataOptions[] PROGMEM =
  "<option value=\"0\">ETC/GMT</option>"
  "<option value=\"1\">Africa/Abidjan</option>"
  "<option value=\"2\">Africa/Accra</option>"
  "<option value=\"3\">Africa/Addis_Ababa</option>"
  "<option value=\"4\">Africa/Algiers</option>"
  "<option value=\"5\">Africa/Asmara</option>"
  "<option value=\"6\">Africa/Bamako</option>"
  "<option value=\"7\">Africa/Bangui</option>"
  "<option value=\"8\">Africa/Banjul</option>"
  "<option value=\"9\">Africa/Bissau</option>"
  "<option value=\"10\">Africa/Blantyre</option>"
  "<option value=\"11\">Africa/Brazzaville</option>"
  "<option value=\"12\">Africa/Bujumbura</option>"
  "<option value=\"13\">Africa/Cairo</option>"
  "<option value=\"14\">Africa/Casablanca</option>"
  "<option value=\"15\">Africa/Ceuta</option>"
  "<option value=\"16\">Africa/Conakry</option>"
  "<option value=\"17\">Africa/Dakar</option>"
  "<option value=\"18\">Africa/Dar_es_Salaam</option>"
  "<option value=\"19\">Africa/Djibouti</option>"
  "<option value=\"20\">Africa/Douala</option>"
  "<option value=\"21\">Africa/El_Aaiun</option>"
  "<option value=\"22\">Africa/Freetown</option>"
  "<option value=\"23\">Africa/Gaborone</option>"
  "<option value=\"24\">Africa/Harare</option>"
  "<option value=\"25\">Africa/Johannesburg</option>"
  "<option value=\"26\">Africa/Juba</option>"
  "<option value=\"27\">Africa/Kampala</option>"
  "<option value=\"28\">Africa/Khartoum</option>"
  "<option value=\"29\">Africa/Kigali</option>"
  "<option value=\"30\">Africa/Kinshasa</option>"
  "<option value=\"31\">Africa/Lagos</option>"
  "<option value=\"32\">Africa/Libreville</option>"
  "<option value=\"33\">Africa/Lome</option>"
  "<option value=\"34\">Africa/Luanda</option>"
  "<option value=\"35\">Africa/Lubumbashi</option>"
  "<option value=\"36\">Africa/Lusaka</option>"
  "<option value=\"37\">Africa/Malabo</option>"
  "<option value=\"38\">Africa/Maputo</option>"
  "<option value=\"39\">Africa/Maseru</option>"
  "<option value=\"40\">Africa/Mbabane</option>"
  "<option value=\"41\">Africa/Mogadishu</option>"
  "<option value=\"42\">Africa/Monrovia</option>"
  "<option value=\"43\">Africa/Nairobi</option>"
  "<option value=\"44\">Africa/Ndjamena</option>"
  "<option value=\"45\">Africa/Niamey</option>"
  "<option value=\"46\">Africa/Nouakchott</option>"
  "<option value=\"47\">Africa/Ouagadougou</option>"
  "<option value=\"48\">Africa/Porto-Novo</option>"
  "<option value=\"49\">Africa/Sao_Tome</option>"
  "<option value=\"50\">Africa/Tripoli</option>"
  "<option value=\"51\">Africa/Tunis</option>"
  "<option value=\"52\">Africa/Windhoek</option>"
  "<option value=\"53\">America/Adak</option>"
  "<option value=\"54\">America/Anchorage</option>"
  "<option value=\"55\">America/Anguilla</option>"
  "<option value=\"56\">America/Antigua</option>"
  "<option value=\"57\">America/Araguaina</option>"
  "<option value=\"58\">America/Argentina/Buenos_Aires</option>"
  "<option value=\"59\">America/Argentina/Catamarca</option>"
  "<option value=\"60\">America/Argentina/Cordoba</option>"
  "<option value=\"61\">America/Argentina/Jujuy</option>"
  "<option value=\"62\">America/Argentina/La_Rioja</option>"
  "<option value=\"63\">America/Argentina/Mendoza</option>"
  "<option value=\"64\">America/Argentina/Rio_Gallegos</option>"
  "<option value=\"65\">America/Argentina/Salta</option>"
  "<option value=\"66\">America/Argentina/San_Juan</option>"
  "<option value=\"67\">America/Argentina/San_Luis</option>"
  "<option value=\"68\">America/Argentina/Tucuman</option>"
  "<option value=\"69\">America/Argentina/Ushuaia</option>"
  "<option value=\"70\">America/Aruba</option>"
  "<option value=\"71\">America/Asuncion</option>"
  "<option value=\"72\">America/Atikokan</option>"
  "<option value=\"73\">America/Bahia</option>"
  "<option value=\"74\">America/Bahia_Banderas</option>"
  "<option value=\"75\">America/Barbados</option>"
  "<option value=\"76\">America/Belem</option>"
  "<option value=\"77\">America/Belize</option>"
  "<option value=\"78\">America/Blanc-Sablon</option>"
  "<option value=\"79\">America/Boa_Vista</option>"
  "<option value=\"80\">America/Bogota</option>"
  "<option value=\"81\">America/Boise</option>"
  "<option value=\"82\">America/Cambridge_Bay</option>"
  "<option value=\"83\">America/Campo_Grande</option>"
  "<option value=\"84\">America/Cancun</option>"
  "<option value=\"85\">America/Caracas</option>"
  "<option value=\"86\">America/Cayenne</option>"
  "<option value=\"87\">America/Cayman</option>"
  "<option value=\"88\">America/Chicago</option>"
  "<option value=\"89\">America/Chihuahua</option>"
  "<option value=\"90\">America/Costa_Rica</option>"
  "<option value=\"91\">America/Creston</option>"
  "<option value=\"92\">America/Cuiaba</option>"
  "<option value=\"93\">America/Curacao</option>"
  "<option value=\"94\">America/Danmarkshavn</option>"
  "<option value=\"95\">America/Dawson</option>"
  "<option value=\"96\">America/Dawson_Creek</option>"
  "<option value=\"97\">America/Denver</option>"
  "<option value=\"98\">America/Detroit</option>"
  "<option value=\"99\">America/Dominica</option>"
  "<option value=\"100\">America/Edmonton</option>"
  "<option value=\"101\">America/Eirunepe</option>"
  "<option value=\"102\">America/El_Salvador</option>"
  "<option value=\"103\">America/Fortaleza</option>"
  "<option value=\"104\">America/Fort_Nelson</option>"
  "<option value=\"105\">America/Glace_Bay</option>"
  "<option value=\"106\">America/Godthab</option>"
  "<option value=\"107\">America/Goose_Bay</option>"
  "<option value=\"108\">America/Grand_Turk</option>"
  "<option value=\"109\">America/Grenada</option>"
  "<option value=\"110\">America/Guadeloupe</option>"
  "<option value=\"111\">America/Guatemala</option>"
  "<option value=\"112\">America/Guayaquil</option>"
  "<option value=\"113\">America/Guyana</option>"
  "<option value=\"114\">America/Halifax</option>"
  "<option value=\"115\">America/Havana</option>"
  "<option value=\"116\">America/Hermosillo</option>"
  "<option value=\"117\">America/Indiana/Indianapolis</option>"
  "<option value=\"118\">America/Indiana/Knox</option>"
  "<option value=\"119\">America/Indiana/Marengo</option>"
  "<option value=\"120\">America/Indiana/Petersburg</option>"
  "<option value=\"121\">America/Indiana/Tell_City</option>"
  "<option value=\"122\">America/Indiana/Vevay</option>"
  "<option value=\"123\">America/Indiana/Vincennes</option>"
  "<option value=\"124\">America/Indiana/Winamac</option>"
  "<option value=\"125\">America/Inuvik</option>"
  "<option value=\"126\">America/Iqaluit</option>"
  "<option value=\"127\">America/Jamaica</option>"
  "<option value=\"128\">America/Juneau</option>"
  "<option value=\"129\">America/Kentucky/Louisville</option>"
  "<option value=\"130\">America/Kentucky/Monticello</option>"
  "<option value=\"131\">America/Kralendijk</option>"
  "<option value=\"132\">America/La_Paz</option>"
  "<option value=\"133\">America/Lima</option>"
  "<option value=\"134\">America/Los_Angeles</option>"
  "<option value=\"135\">America/Lower_Princes</option>"
  "<option value=\"136\">America/Maceio</option>"
  "<option value=\"137\">America/Managua</option>"
  "<option value=\"138\">America/Manaus</option>"
  "<option value=\"139\">America/Marigot</option>"
  "<option value=\"140\">America/Martinique</option>"
  "<option value=\"141\">America/Matamoros</option>"
  "<option value=\"142\">America/Mazatlan</option>"
  "<option value=\"143\">America/Menominee</option>"
  "<option value=\"144\">America/Merida</option>"
  "<option value=\"145\">America/Metlakatla</option>"
  "<option value=\"146\">America/Mexico_City</option>"
  "<option value=\"147\">America/Miquelon</option>"
  "<option value=\"148\">America/Moncton</option>"
  "<option value=\"149\">America/Monterrey</option>"
  "<option value=\"150\">America/Montevideo</option>"
  "<option value=\"151\">America/Montreal</option>"
  "<option value=\"152\">America/Montserrat</option>"
  "<option value=\"153\">America/Nassau</option>"
  "<option value=\"154\">America/New_York</option>"
  "<option value=\"155\">America/Nipigon</option>"
  "<option value=\"156\">America/Nome</option>"
  "<option value=\"157\">America/Noronha</option>"
  "<option value=\"158\">America/North_Dakota/Beulah</option>"
  "<option value=\"159\">America/North_Dakota/Center</option>"
  "<option value=\"160\">America/North_Dakota/New_Salem</option>"
  "<option value=\"161\">America/Nuuk</option>"
  "<option value=\"162\">America/Ojinaga</option>"
  "<option value=\"163\">America/Panama</option>"
  "<option value=\"164\">America/Pangnirtung</option>"
  "<option value=\"165\">America/Paramaribo</option>"
  "<option value=\"166\">America/Phoenix</option>"
  "<option value=\"167\">America/Port-au-Prince</option>"
  "<option value=\"168\">America/Port_of_Spain</option>"
  "<option value=\"169\">America/Porto_Velho</option>"
  "<option value=\"170\">America/Puerto_Rico</option>"
  "<option value=\"171\">America/Punta_Arenas</option>"
  "<option value=\"172\">America/Rainy_River</option>"
  "<option value=\"173\">America/Rankin_Inlet</option>"
  "<option value=\"174\">America/Recife</option>"
  "<option value=\"175\">America/Regina</option>"
  "<option value=\"176\">America/Resolute</option>"
  "<option value=\"177\">America/Rio_Branco</option>"
  "<option value=\"178\">America/Santarem</option>"
  "<option value=\"179\">America/Santiago</option>"
  "<option value=\"180\">America/Santo_Domingo</option>"
  "<option value=\"181\">America/Sao_Paulo</option>"
  "<option value=\"182\">America/Scoresbysund</option>"
  "<option value=\"183\">America/Sitka</option>"
  "<option value=\"184\">America/St_Barthelemy</option>"
  "<option value=\"185\">America/St_Johns</option>"
  "<option value=\"186\">America/St_Kitts</option>"
  "<option value=\"187\">America/St_Lucia</option>"
  "<option value=\"188\">America/St_Thomas</option>"
  "<option value=\"189\">America/St_Vincent</option>"
  "<option value=\"190\">America/Swift_Current</option>"
  "<option value=\"191\">America/Tegucigalpa</option>"
  "<option value=\"192\">America/Thule</option>"
  "<option value=\"193\">America/Thunder_Bay</option>"
  "<option value=\"194\">America/Tijuana</option>"
  "<option value=\"195\">America/Toronto</option>"
  "<option value=\"196\">America/Tortola</option>"
  "<option value=\"197\">America/Vancouver</option>"
  "<option value=\"198\">America/Whitehorse</option>"
  "<option value=\"199\">America/Winnipeg</option>"
  "<option value=\"200\">America/Yakutat</option>"
  "<option value=\"201\">America/Yellowknife</option>"
  "<option value=\"202\">Antarctica/Casey</option>"
  "<option value=\"203\">Antarctica/Davis</option>"
  "<option value=\"204\">Antarctica/DumontDUrville</option>"
  "<option value=\"205\">Antarctica/Macquarie</option>"
  "<option value=\"206\">Antarctica/Mawson</option>"
  "<option value=\"207\">Antarctica/McMurdo</option>"
  "<option value=\"208\">Antarctica/Palmer</option>"
  "<option value=\"209\">Antarctica/Rothera</option>"
  "<option value=\"210\">Antarctica/Syowa</option>"
  "<option value=\"211\">Antarctica/Troll</option>"
  "<option value=\"212\">Antarctica/Vostok</option>"
  "<option value=\"213\">Arctic/Longyearbyen</option>"
  "<option value=\"214\">Asia/Aden</option>"
  "<option value=\"215\">Asia/Almaty</option>"
  "<option value=\"216\">Asia/Amman</option>"
  "<option value=\"217\">Asia/Anadyr</option>"
  "<option value=\"218\">Asia/Aqtau</option>"
  "<option value=\"219\">Asia/Aqtobe</option>"
  "<option value=\"220\">Asia/Ashgabat</option>"
  "<option value=\"221\">Asia/Atyrau</option>"
  "<option value=\"222\">Asia/Baghdad</option>"
  "<option value=\"223\">Asia/Bahrain</option>"
  "<option value=\"224\">Asia/Baku</option>"
  "<option value=\"225\">Asia/Bangkok</option>"
  "<option value=\"226\">Asia/Barnaul</option>"
  "<option value=\"227\">Asia/Beirut</option>"
  "<option value=\"228\">Asia/Bishkek</option>"
  "<option value=\"229\">Asia/Brunei</option>"
  "<option value=\"230\">Asia/Chita</option>"
  "<option value=\"231\">Asia/Choibalsan</option>"
  "<option value=\"232\">Asia/Colombo</option>"
  "<option value=\"233\">Asia/Damascus</option>"
  "<option value=\"234\">Asia/Dhaka</option>"
  "<option value=\"235\">Asia/Dili</option>"
  "<option value=\"236\">Asia/Dubai</option>"
  "<option value=\"237\">Asia/Dushanbe</option>"
  "<option value=\"238\">Asia/Famagusta</option>"
  "<option value=\"239\">Asia/Gaza</option>"
  "<option value=\"240\">Asia/Hebron</option>"
  "<option value=\"241\">Asia/Ho_Chi_Minh</option>"
  "<option value=\"242\">Asia/Hong_Kong</option>"
  "<option value=\"243\">Asia/Hovd</option>"
  "<option value=\"244\">Asia/Irkutsk</option>"
  "<option value=\"245\">Asia/Jakarta</option>"
  "<option value=\"246\">Asia/Jayapura</option>"
  "<option value=\"247\">Asia/Jerusalem</option>"
  "<option value=\"248\">Asia/Kabul</option>"
  "<option value=\"249\">Asia/Kamchatka</option>"
  "<option value=\"250\">Asia/Karachi</option>"
  "<option value=\"251\">Asia/Kathmandu</option>"
  "<option value=\"252\">Asia/Khandyga</option>"
  "<option value=\"253\">Asia/Kolkata</option>"
  "<option value=\"254\">Asia/Krasnoyarsk</option>"
  "<option value=\"255\">Asia/Kuala_Lumpur</option>"
  "<option value=\"256\">Asia/Kuching</option>"
  "<option value=\"257\">Asia/Kuwait</option>"
  "<option value=\"258\">Asia/Macau</option>"
  "<option value=\"259\">Asia/Magadan</option>"
  "<option value=\"260\">Asia/Makassar</option>"
  "<option value=\"261\">Asia/Manila</option>"
  "<option value=\"262\">Asia/Muscat</option>"
  "<option value=\"263\">Asia/Nicosia</option>"
  "<option value=\"264\">Asia/Novokuznetsk</option>"
  "<option value=\"265\">Asia/Novosibirsk</option>"
  "<option value=\"266\">Asia/Omsk</option>"
  "<option value=\"267\">Asia/Oral</option>"
  "<option value=\"268\">Asia/Phnom_Penh</option>"
  "<option value=\"269\">Asia/Pontianak</option>"
  "<option value=\"270\">Asia/Pyongyang</option>"
  "<option value=\"271\">Asia/Qatar</option>"
  "<option value=\"272\">Asia/Qyzylorda</option>"
  "<option value=\"273\">Asia/Riyadh</option>"
  "<option value=\"274\">Asia/Sakhalin</option>"
  "<option value=\"275\">Asia/Samarkand</option>"
  "<option value=\"276\">Asia/Seoul</option>"
  "<option value=\"277\">Asia/Shanghai</option>"
  "<option value=\"278\">Asia/Singapore</option>"
  "<option value=\"279\">Asia/Srednekolymsk</option>"
  "<option value=\"280\">Asia/Taipei</option>"
  "<option value=\"281\">Asia/Tashkent</option>"
  "<option value=\"282\">Asia/Tbilisi</option>"
  "<option value=\"283\">Asia/Tehran</option>"
  "<option value=\"284\">Asia/Thimphu</option>"
  "<option value=\"285\">Asia/Tokyo</option>"
  "<option value=\"286\">Asia/Tomsk</option>"
  "<option value=\"287\">Asia/Ulaanbaatar</option>"
  "<option value=\"288\">Asia/Urumqi</option>"
  "<option value=\"289\">Asia/Ust-Nera</option>"
  "<option value=\"290\">Asia/Vientiane</option>"
  "<option value=\"291\">Asia/Vladivostok</option>"
  "<option value=\"292\">Asia/Yakutsk</option>"
  "<option value=\"293\">Asia/Yangon</option>"
  "<option value=\"294\">Asia/Yekaterinburg</option>"
  "<option value=\"295\">Asia/Yerevan</option>"
  "<option value=\"296\">Atlantic/Azores</option>"
  "<option value=\"297\">Atlantic/Bermuda</option>"
  "<option value=\"298\">Atlantic/Canary</option>"
  "<option value=\"299\">Atlantic/Cape_Verde</option>"
  "<option value=\"300\">Atlantic/Faroe</option>"
  "<option value=\"301\">Atlantic/Madeira</option>"
  "<option value=\"302\">Atlantic/Reykjavik</option>"
  "<option value=\"303\">Atlantic/South_Georgia</option>"
  "<option value=\"304\">Atlantic/Stanley</option>"
  "<option value=\"305\">Atlantic/St_Helena</option>"
  "<option value=\"306\">Australia/Adelaide</option>"
  "<option value=\"307\">Australia/Brisbane</option>"
  "<option value=\"308\">Australia/Broken_Hill</option>"
  "<option value=\"309\">Australia/Currie</option>"
  "<option value=\"310\">Australia/Darwin</option>"
  "<option value=\"311\">Australia/Eucla</option>"
  "<option value=\"312\">Australia/Hobart</option>"
  "<option value=\"313\">Australia/Lindeman</option>"
  "<option value=\"314\">Australia/Lord_Howe</option>"
  "<option value=\"315\">Australia/Melbourne</option>"
  "<option value=\"316\">Australia/Perth</option>"
  "<option value=\"317\">Australia/Sydney</option>"
  "<option value=\"318\">Europe/Amsterdam</option>"
  "<option value=\"319\">Europe/Andorra</option>"
  "<option value=\"320\">Europe/Astrakhan</option>"
  "<option value=\"321\">Europe/Athens</option>"
  "<option value=\"322\">Europe/Belgrade</option>"
  "<option value=\"323\">Europe/Berlin</option>"
  "<option value=\"324\">Europe/Bratislava</option>"
  "<option value=\"325\">Europe/Brussels</option>"
  "<option value=\"326\">Europe/Bucharest</option>"
  "<option value=\"327\">Europe/Budapest</option>"
  "<option value=\"328\">Europe/Busingen</option>"
  "<option value=\"329\">Europe/Chisinau</option>"
  "<option value=\"330\">Europe/Copenhagen</option>"
  "<option value=\"331\">Europe/Dublin</option>"
  "<option value=\"332\">Europe/Gibraltar</option>"
  "<option value=\"333\">Europe/Guernsey</option>"
  "<option value=\"334\">Europe/Helsinki</option>"
  "<option value=\"335\">Europe/Isle_of_Man</option>"
  "<option value=\"336\">Europe/Istanbul</option>"
  "<option value=\"337\">Europe/Jersey</option>"
  "<option value=\"338\">Europe/Kaliningrad</option>"
  "<option value=\"339\">Europe/Kiev</option>"
  "<option value=\"340\">Europe/Kirov</option>"
  "<option value=\"341\">Europe/Lisbon</option>"
  "<option value=\"342\">Europe/Ljubljana</option>"
  "<option value=\"343\">Europe/London</option>"
  "<option value=\"344\">Europe/Luxembourg</option>"
  "<option value=\"345\">Europe/Madrid</option>"
  "<option value=\"346\">Europe/Malta</option>"
  "<option value=\"347\">Europe/Mariehamn</option>"
  "<option value=\"348\">Europe/Minsk</option>"
  "<option value=\"349\">Europe/Monaco</option>"
  "<option value=\"350\">Europe/Moscow</option>"
  "<option value=\"351\">Europe/Oslo</option>"
  "<option value=\"352\">Europe/Paris</option>"
  "<option value=\"353\">Europe/Podgorica</option>"
  "<option value=\"354\">Europe/Prague</option>"
  "<option value=\"355\">Europe/Riga</option>"
  "<option value=\"356\">Europe/Rome</option>"
  "<option value=\"357\">Europe/Samara</option>"
  "<option value=\"358\">Europe/San_Marino</option>"
  "<option value=\"359\">Europe/Sarajevo</option>"
  "<option value=\"360\">Europe/Saratov</option>"
  "<option value=\"361\">Europe/Simferopol</option>"
  "<option value=\"362\">Europe/Skopje</option>"
  "<option value=\"363\">Europe/Sofia</option>"
  "<option value=\"364\">Europe/Stockholm</option>"
  "<option value=\"365\">Europe/Tallinn</option>"
  "<option value=\"366\">Europe/Tirane</option>"
  "<option value=\"367\">Europe/Ulyanovsk</option>"
  "<option value=\"368\">Europe/Uzhgorod</option>"
  "<option value=\"369\">Europe/Vaduz</option>"
  "<option value=\"370\">Europe/Vatican</option>"
  "<option value=\"371\">Europe/Vienna</option>"
  "<option value=\"372\">Europe/Vilnius</option>"
  "<option value=\"373\">Europe/Volgograd</option>"
  "<option value=\"374\">Europe/Warsaw</option>"
  "<option value=\"375\">Europe/Zagreb</option>"
  "<option value=\"376\">Europe/Zaporozhye</option>"
  "<option value=\"377\">Europe/Zurich</option>"
  "<option value=\"378\">Indian/Antananarivo</option>"
  "<option value=\"379\">Indian/Chagos</option>"
  "<option value=\"380\">Indian/Christmas</option>"
  "<option value=\"381\">Indian/Cocos</option>"
  "<option value=\"382\">Indian/Comoro</option>"
  "<option value=\"383\">Indian/Kerguelen</option>"
  "<option value=\"384\">Indian/Mahe</option>"
  "<option value=\"385\">Indian/Maldives</option>"
  "<option value=\"386\">Indian/Mauritius</option>"
  "<option value=\"387\">Indian/Mayotte</option>"
  "<option value=\"388\">Indian/Reunion</option>"
  "<option value=\"389\">Pacific/Apia</option>"
  "<option value=\"390\">Pacific/Auckland</option>"
  "<option value=\"391\">Pacific/Bougainville</option>"
  "<option value=\"392\">Pacific/Chatham</option>"
  "<option value=\"393\">Pacific/Chuuk</option>"
  "<option value=\"394\">Pacific/Easter</option>"
  "<option value=\"395\">Pacific/Efate</option>"
  "<option value=\"396\">Pacific/Enderbury</option>"
  "<option value=\"397\">Pacific/Fakaofo</option>"
  "<option value=\"398\">Pacific/Fiji</option>"
  "<option value=\"399\">Pacific/Funafuti</option>"
  "<option value=\"400\">Pacific/Galapagos</option>"
  "<option value=\"401\">Pacific/Gambier</option>"
  "<option value=\"402\">Pacific/Guadalcanal</option>"
  "<option value=\"403\">Pacific/Guam</option>"
  "<option value=\"404\">Pacific/Honolulu</option>"
  "<option value=\"405\">Pacific/Kiritimati</option>"
  "<option value=\"406\">Pacific/Kosrae</option>"
  "<option value=\"407\">Pacific/Kwajalein</option>"
  "<option value=\"408\">Pacific/Majuro</option>"
  "<option value=\"409\">Pacific/Marquesas</option>"
  "<option value=\"410\">Pacific/Midway</option>"
  "<option value=\"411\">Pacific/Nauru</option>"
  "<option value=\"412\">Pacific/Niue</option>"
  "<option value=\"413\">Pacific/Norfolk</option>"
  "<option value=\"414\">Pacific/Noumea</option>"
  "<option value=\"415\">Pacific/Pago_Pago</option>"
  "<option value=\"416\">Pacific/Palau</option>"
  "<option value=\"417\">Pacific/Pitcairn</option>"
  "<option value=\"418\">Pacific/Pohnpei</option>"
  "<option value=\"419\">Pacific/Port_Moresby</option>"
  "<option value=\"420\">Pacific/Rarotonga</option>"
  "<option value=\"421\">Pacific/Saipan</option>"
  "<option value=\"422\">Pacific/Tahiti</option>"
  "<option value=\"423\">Pacific/Tarawa</option>"
  "<option value=\"424\">Pacific/Tongatapu</option>"
  "<option value=\"425\">Pacific/Wake</option>"
  "<option value=\"426\">Pacific/Wallis</option>"
  "<option value=\"427\">Etc/GMT-0</option>"
  "<option value=\"428\">Etc/GMT-1</option>"
  "<option value=\"429\">Etc/GMT-2</option>"
  "<option value=\"430\">Etc/GMT-3</option>"
  "<option value=\"431\">Etc/GMT-4</option>"
  "<option value=\"432\">Etc/GMT-5</option>"
  "<option value=\"433\">Etc/GMT-6</option>"
  "<option value=\"434\">Etc/GMT-7</option>"
  "<option value=\"435\">Etc/GMT-8</option>"
  "<option value=\"436\">Etc/GMT-9</option>"
  "<option value=\"437\">Etc/GMT-10</option>"
  "<option value=\"438\">Etc/GMT-11</option>"
  "<option value=\"439\">Etc/GMT-12</option>"
  "<option value=\"440\">Etc/GMT-13</option>"
  "<option value=\"441\">Etc/GMT-14</option>"
  "<option value=\"442\">Etc/GMT0</option>"
  "<option value=\"443\">Etc/GMT+0</option>"
  "<option value=\"444\">Etc/GMT+1</option>"
  "<option value=\"445\">Etc/GMT+2</option>"
  "<option value=\"446\">Etc/GMT+3</option>"
  "<option value=\"447\">Etc/GMT+4</option>"
  "<option value=\"448\">Etc/GMT+5</option>"
  "<option value=\"449\">Etc/GMT+6</option>"
  "<option value=\"450\">Etc/GMT+7</option>"
  "<option value=\"451\">Etc/GMT+8</option>"
  "<option value=\"452\">Etc/GMT+9</option>"
  "<option value=\"453\">Etc/GMT+10</option>"
  "<option value=\"454\">Etc/GMT+11</option>"
  "<option value=\"455\">Etc/GMT+12</option>"
  "<option value=\"456\">Etc/UCT</option>"
  "<option value=\"457\">Etc/UTC</option>"
  "<option value=\"458\">Etc/Greenwich</option>"
  "<option value=\"459\">Etc/Universal</option>"
  "<option value=\"460\">Etc/Zulu</option>";
