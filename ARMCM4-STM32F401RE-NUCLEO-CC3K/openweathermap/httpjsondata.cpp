#include "httpjsondata.h"
#include "httptcpconnection.h"
#include "ch.h"
#include "chprintf.h"

#include <string.h>

typedef enum {
    START,
    SECTION_NAME_FOUND,
    VAR_NAME_FOUND,
    VALUE_FOUND
}detect_states;


int HttpJsonData::jsonGetVal(jsmntok_t * tokens, const char * js, const char * section, const char * varname, char * value, size_t len )
{

    int i, strsiz, curItem = 0;
    jsmntok_t * t;
    detect_states sw_state = START;

    if ((tokens == NULL) ||
        (section == NULL) ||
        (varname == NULL) ||
        (value == NULL)) return -1;

    for (i = 1; tokens[i].end < tokens[0].end; i++)
    {
        if (tokens[i].type == JSMN_STRING || tokens[i].type == JSMN_PRIMITIVE)
        {
            strsiz = tokens[i].end - tokens[i].start;
            if (len < strsiz) return -1;
            memset(value, 0, len);

            strncpy(value, js + tokens[i].start, strsiz );
            if ((curItem % 2) == 0)
            {
                if ((sw_state == SECTION_NAME_FOUND) && ( strcmp(value, varname) == 0))
                    sw_state = VAR_NAME_FOUND;
            }
            else
            {
                if (sw_state == VAR_NAME_FOUND)
                    return 0;
            }

            curItem++;

        } else if (tokens[i].type == JSMN_ARRAY)
        {
            // TODO: handle arrays, otherwise last varname will persist
        } else if (tokens[i].type == JSMN_OBJECT)
        {
            if ((sw_state == START) && (strcmp(value, section) == 0))
                sw_state = SECTION_NAME_FOUND;

            curItem = 0;
        }
        else
        {
            // TOKEN_PRINT(tokens[i]); // discard umatched..
        }
    }

    return -1; // Not found
}


HttpJsonData::HttpJsonData()
{
}

HttpJsonData::~HttpJsonData()
{
}

HTTP_RESULT HttpJsonData::sendHeader(HttpTcpConnection * connection)
{
    return HTTP_OK;
}

HTTP_RESULT HttpJsonData::handleData(HttpTcpConnection * connection, HttpResponseInfo & info)
{

  char data[1024];
  size_t datasiz = sizeof(data);
  jsmn_parser p;
  jsmntok_t t[100];
  char tmp[100]; // temp storage for values read


  // Prime the connection to receive chunked transfers
  if (connection)
  {
   connection->setChunked(info.transferEncoding == HDR_TE_CHUNKED);

   if (connection->receive_data(data, datasiz ) == HTTP_OK)
   {
       // process data
       jsmn_init(&p);
       //processData(data, datasiz);
       if (jsmn_parse(&p, data, datasiz, t, 100) >= 0)
       {
         // process_json(t, js, r);
         if (jsonGetVal(t, data, "weather", "description",  tmp,  sizeof(tmp)) == 0)
             DBG("Weather description = [%s]\n", tmp);
         if (jsonGetVal(t, data, "main", "temp", tmp, sizeof(tmp)) == 0)
             DBG("Temperature = [%s]\n", tmp);
         if (jsonGetVal(t, data, "main", "pressure", tmp, sizeof(tmp)) == 0)
             DBG("Pressure = [%s]\n", tmp);
         if (jsonGetVal(t, data, "main", "humidity", tmp, sizeof(tmp)) == 0)
             DBG("Humidity = [%s]\n", tmp);
       }
   }
  }

  return HTTP_OK;
}
