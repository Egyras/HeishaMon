#include "htmlcode.h"
#include "version.h"

static char _htmlBuf[256];

String Html::leftMenu(String currentUrl) {
    String html = F("<div  id='leftMenu' class='w3-sidebar w3-bar-block w3-card w3-animate-left' style='display:none'>");
    if (currentUrl != F("/")) html += menuItem(F("Home"), F("/"));
    if (currentUrl != F("/topics")) html += menuItem(F("Select topics"), F("/topics"));
    if (currentUrl != F("/settings")) html += menuItem(F("Settings"), F("/settings"));
    html += menuItem(F("Firmware"), F("/firmware"));
    html += menuItem(F("Reboot"), F("/reboot"));
    html += menuItem(F("Toggle MQTT log"), F("/togglelog"));
    html += menuItem(F("Toggle hexdump log"), F("/togglehexdump"));
    html += F("<hr><div class='w3-text-grey'>Version: ");
    html += heishamon_version;
    html += F("<br><a href=\"https://github.com/Egyras/HeishaMon\">Heishamon software</a></div><hr></div>");

    return html;
}

String Html::menuItem(String name, String url) {
    snprintf(
        _htmlBuf,
        sizeof(_htmlBuf),
        "<a href='%s' class='w3-bar-item w3-button'>%s</a>",
        url.c_str(),
        name.c_str()
    );

    return String(_htmlBuf);
}

String Html::textBox(String name, String value, const char* type, const char* additionalAttrs)
{
  snprintf(
    _htmlBuf,
    sizeof(_htmlBuf),
    "<input type='%s' name='%s' value='%s' %s/>",
    type,
    name.c_str(),
    value.c_str(),
    additionalAttrs
    );

  return String(_htmlBuf);
}

String Html::checkbox(String name, bool value, const char* additionalAttrs) {
  const char* checked = value ? "checked" : "";
  snprintf(
    _htmlBuf,
    sizeof(_htmlBuf),
    "<input type='checkbox' name='%s' value='enabled' %s %s/>",
    name.c_str(),
    checked,
    additionalAttrs);

  return String(_htmlBuf);
}

String Html::radioButton(String name, String label, int value, int currentValue) {
  const char* checked = (value == currentValue) ? "checked" : "";
  snprintf(
    _htmlBuf,
    sizeof(_htmlBuf),
    "<input type='radio' name='%s' value='%d' %s/><label for='%s'>%s</label>",
    name.c_str(),
    value,
    checked,
    name.c_str(),
    label.c_str());

  return String(_htmlBuf);
}

String Html::option(String value, String label, bool selected) {
  const char* selectedStr = selected ? "selected" : "";
  snprintf(
    _htmlBuf,
    sizeof(_htmlBuf),
    "<option value='%s' %s/>%s</option>",
    value.c_str(),
    selectedStr,
    label.c_str());

  return String(_htmlBuf);
}
