#include "../include/VCParser.h"

int main() {
    Card* card = NULL;
       VCardErrorCode status = createCard("testCard.vcf", &card);

    if (status != OK) {
        printf("Error creating card: %d\n", status);
        return 1;
    }

    char* cardStr = cardToString(card);
    printf("%s", cardStr);

    return 0;
}

VCardErrorCode writeCard(const char* fileName, const Card* obj) {

    if (fileName == NULL || obj == NULL) {
        return WRITE_ERROR;
    }

    FILE* file = fopen(fileName, "w");
    if (file == NULL) {
        return WRITE_ERROR;
    }

    if (fprintf(file, "BEGIN:VCARD\r\n") < 0) {
        fclose(file);
        return WRITE_ERROR;
    }

    if (fprintf(file, "VERSION:4.0\r\n") < 0) {
        fclose(file);
        return WRITE_ERROR;
    }

    if (obj->fn != NULL) {

        if (obj->fn->group != NULL && strlen(obj->fn->group) > 0) {
            if (fprintf(file, "%s.", obj->fn->group) < 0) {
                fclose(file);
                return WRITE_ERROR;
            }
        }

        if (fprintf(file, "FN") < 0) {
            fclose(file);
            return WRITE_ERROR;
        }

        if (obj->fn->parameters != NULL && obj->fn->parameters->length > 0) {
            Node* paramNode = obj->fn->parameters->head;
            while (paramNode != NULL) {
                Parameter* param = (Parameter*)paramNode->data;
                if (param != NULL && param->name != NULL && param->value != NULL) {
                    if (fprintf(file, ";%s=%s", param->name, param->value) < 0) {
                        fclose(file);
                        return WRITE_ERROR;
                    }
                }
                paramNode = paramNode->next;
            }
        }

        if (obj->fn->values != NULL && obj->fn->values->head != NULL) {
            char* value = (char*)obj->fn->values->head->data;
            if (value != NULL) {
                if (fprintf(file, ":%s\r\n", value) < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }
            } else {

                fclose(file);
                return WRITE_ERROR;
            }
        } else {

            fclose(file);
            return WRITE_ERROR;
        }
    } else {

        fclose(file);
        return WRITE_ERROR;
    }

    bool nWritten = false;
    if (obj->optionalProperties != NULL) {
        Node* propNode = obj->optionalProperties->head;
        while (propNode != NULL && !nWritten) {
            Property* prop = (Property*)propNode->data;
            if (prop != NULL && prop->name != NULL && strcmp(prop->name, "N") == 0) {

                if (prop->group != NULL && strlen(prop->group) > 0) {
                    if (fprintf(file, "%s.", prop->group) < 0) {
                        fclose(file);
                        return WRITE_ERROR;
                    }
                }

                if (fprintf(file, "%s", prop->name) < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }

                if (prop->parameters != NULL && prop->parameters->length > 0) {
                    Node* paramNode = prop->parameters->head;
                    while (paramNode != NULL) {
                        Parameter* param = (Parameter*)paramNode->data;
                        if (param != NULL && param->name != NULL && param->value != NULL) {
                            if (fprintf(file, ";%s=%s", param->name, param->value) < 0) {
                                fclose(file);
                                return WRITE_ERROR;
                            }
                        }
                        paramNode = paramNode->next;
                    }
                }

                if (fprintf(file, ":") < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }

                Node* valueNode = prop->values->head;
                int valueCount = 0;

                while (valueCount < 5) {
                    char* value = NULL;

                    if (valueNode != NULL) {
                        value = (char*)valueNode->data;
                        valueNode = valueNode->next;
                    }

                    if (fprintf(file, "%s", value != NULL ? value : "") < 0) {
                        fclose(file);
                        return WRITE_ERROR;
                    }

                    if (valueCount < 4) {
                        if (fprintf(file, ";") < 0) {
                            fclose(file);
                            return WRITE_ERROR;
                        }
                    }

                    valueCount++;
                }

                if (fprintf(file, "\r\n") < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }

                nWritten = true;
            }
            propNode = propNode->next;
        }
    }

    if (obj->birthday != NULL) {
        if (fprintf(file, "BDAY") < 0) {
            fclose(file);
            return WRITE_ERROR;
        }

        if (obj->birthday->isText) {
            if (fprintf(file, ";VALUE=text:%s\r\n", obj->birthday->text) < 0) {
                fclose(file);
                return WRITE_ERROR;
            }
        } else {

            if (fprintf(file, ":") < 0) {
                fclose(file);
                return WRITE_ERROR;
            }

            if (obj->birthday->date != NULL) {
                if (fprintf(file, "%s", obj->birthday->date) < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }
            }

            if (obj->birthday->time != NULL && strlen(obj->birthday->time) > 0) {
                if (fprintf(file, "T%s", obj->birthday->time) < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }
            }

            if (obj->birthday->UTC) {
                if (fprintf(file, "Z") < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }
            }

            if (fprintf(file, "\r\n") < 0) {
                fclose(file);
                return WRITE_ERROR;
            }
        }
    }

    if (obj->anniversary != NULL) {
        if (fprintf(file, "ANNIVERSARY") < 0) {
            fclose(file);
            return WRITE_ERROR;
        }

        if (obj->anniversary->isText) {
            if (fprintf(file, ";VALUE=text:%s\r\n", obj->anniversary->text) < 0) {
                fclose(file);
                return WRITE_ERROR;
            }
        } else {

            if (fprintf(file, ":") < 0) {
                fclose(file);
                return WRITE_ERROR;
            }

            if (obj->anniversary->date != NULL) {
                if (fprintf(file, "%s", obj->anniversary->date) < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }
            }

            if (obj->anniversary->time != NULL && strlen(obj->anniversary->time) > 0) {
                if (fprintf(file, "T%s", obj->anniversary->time) < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }
            }

            if (obj->anniversary->UTC) {
                if (fprintf(file, "Z") < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }
            }

            if (fprintf(file, "\r\n") < 0) {
                fclose(file);
                return WRITE_ERROR;
            }
        }
    }

    if (obj->optionalProperties != NULL) {
        Node* propNode = obj->optionalProperties->head;
        while (propNode != NULL) {
            Property* prop = (Property*)propNode->data;

            if (prop != NULL && prop->name != NULL && strcmp(prop->name, "N") != 0) {

                if (prop->group != NULL && strlen(prop->group) > 0) {
                    if (fprintf(file, "%s.", prop->group) < 0) {
                        fclose(file);
                        return WRITE_ERROR;
                    }
                }

                if (fprintf(file, "%s", prop->name) < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }

                if (prop->parameters != NULL && prop->parameters->length > 0) {
                    Node* paramNode = prop->parameters->head;
                    while (paramNode != NULL) {
                        Parameter* param = (Parameter*)paramNode->data;
                        if (param != NULL && param->name != NULL && param->value != NULL) {
                            if (fprintf(file, ";%s=%s", param->name, param->value) < 0) {
                                fclose(file);
                                return WRITE_ERROR;
                            }
                        }
                        paramNode = paramNode->next;
                    }
                }

                if (prop->values != NULL && prop->values->length > 0) {
                    if (fprintf(file, ":") < 0) {
                        fclose(file);
                        return WRITE_ERROR;
                    }

                    if (strcmp(prop->name, "ADR") == 0) {

                        Node* valueNode = prop->values->head;
                        int valueCount = 0;

                        while (valueCount < 7) {
                            char* value = NULL;

                            if (valueNode != NULL) {
                                value = (char*)valueNode->data;
                                valueNode = valueNode->next;
                            }

                            if (fprintf(file, "%s", value != NULL ? value : "") < 0) {
                                fclose(file);
                                return WRITE_ERROR;
                            }

                            if (valueCount < 6) {
                                if (fprintf(file, ";") < 0) {
                                    fclose(file);
                                    return WRITE_ERROR;
                                }
                            }

                            valueCount++;
                        }
                    }
                    else if (strcmp(prop->name, "GEO") == 0) {

                        Node* valueNode = prop->values->head;
                        int valueCount = 0;

                        while (valueCount < 2) {
                            char* value = NULL;

                            if (valueNode != NULL) {
                                value = (char*)valueNode->data;
                                valueNode = valueNode->next;
                            }

                            if (fprintf(file, "%s", value != NULL ? value : "") < 0) {
                                fclose(file);
                                return WRITE_ERROR;
                            }

                            if (valueCount == 0) {
                                if (fprintf(file, ",") < 0) {
                                    fclose(file);
                                    return WRITE_ERROR;
                                }
                            }

                            valueCount++;
                        }
                    }
                    else {

                        Node* valueNode = prop->values->head;

                        while (valueNode != NULL) {
                            char* value = (char*)valueNode->data;

                            if (fprintf(file, "%s", value != NULL ? value : "") < 0) {
                                fclose(file);
                                return WRITE_ERROR;
                            }

                            if (valueNode->next != NULL) {
                                if (fprintf(file, ";") < 0) {
                                    fclose(file);
                                    return WRITE_ERROR;
                                }
                            }

                            valueNode = valueNode->next;
                        }
                    }
                } else {

                    if (fprintf(file, ":") < 0) {
                        fclose(file);
                        return WRITE_ERROR;
                    }
                }

                if (fprintf(file, "\r\n") < 0) {
                    fclose(file);
                    return WRITE_ERROR;
                }
            }

            propNode = propNode->next;
        }
    }

    if (fprintf(file, "END:VCARD\r\n") < 0) {
        fclose(file);
        return WRITE_ERROR;
    }

    fclose(file);
    return OK;
}

VCardErrorCode validateCard(const Card* obj) {

    if (obj == NULL) {
        return INV_CARD;
    }

    if (obj->fn == NULL) {
        return INV_CARD;
    }

    if (obj->fn->name == NULL || obj->fn->group == NULL || 
        obj->fn->parameters == NULL || obj->fn->values == NULL) {
        return INV_CARD;
    }

    if (obj->fn->values->length == 0 || obj->fn->values->head == NULL) {
        return INV_PROP;
    }

    Node* paramNode = obj->fn->parameters->head;
    while (paramNode != NULL) {
        Parameter* param = (Parameter*)paramNode->data;
        if (param == NULL || param->name == NULL || param->value == NULL || 
            strlen(param->name) == 0 || strlen(param->value) == 0) {
            return INV_PROP;
        }
        paramNode = paramNode->next;
    }

    if (obj->optionalProperties == NULL) {
        return INV_CARD;
    }

    if (obj->birthday != NULL) {

        if (obj->birthday->date == NULL || obj->birthday->text == NULL) {
            return INV_DT;
        }

        if (obj->birthday->isText) {

            if (strlen(obj->birthday->text) == 0) {
                return INV_DT;
            }

            if (strlen(obj->birthday->date) != 0) {
                return INV_DT;
            }

            if (obj->birthday->time != NULL && strlen(obj->birthday->time) != 0) {
                return INV_DT;
            }

            if (obj->birthday->UTC) {
                return INV_DT;
            }
        } else {

            if (strlen(obj->birthday->text) != 0) {
                return INV_DT;
            }

            if (obj->birthday->UTC && (obj->birthday->time == NULL || strlen(obj->birthday->time) == 0)) {
                return INV_DT;
            }
        }
    }

    if (obj->anniversary != NULL) {

        if (obj->anniversary->date == NULL || obj->anniversary->text == NULL) {
            return INV_DT;
        }

        if (obj->anniversary->isText) {

            if (strlen(obj->anniversary->text) == 0) {
                return INV_DT;
            }

            if (strlen(obj->anniversary->date) != 0) {
                return INV_DT;
            }

            if (obj->anniversary->time != NULL && strlen(obj->anniversary->time) != 0) {
                return INV_DT;
            }

            if (obj->anniversary->UTC) {
                return INV_DT;
            }
        } else {

            if (strlen(obj->anniversary->text) != 0) {
                return INV_DT;
            }

            if (obj->anniversary->UTC && (obj->anniversary->time == NULL || strlen(obj->anniversary->time) == 0)) {
                return INV_DT;
            }
        }
    }

    int kindCount = 0;
    int nCount = 0;
    int genderCount = 0;
    int orgCount = 0;
    int categoriesCount = 0;
    int prodidCount = 0;
    int revCount = 0;
    int uidCount = 0;

    Node* propNode = obj->optionalProperties->head;
    while (propNode != NULL) {
        Property* prop = (Property*)propNode->data;

        if (prop == NULL || prop->name == NULL || prop->group == NULL || 
            prop->parameters == NULL || prop->values == NULL) {
            return INV_CARD;
        }

        if (strcmp(prop->name, "VERSION") == 0) {
            return INV_CARD;
        }

        if (strcmp(prop->name, "BDAY") == 0 || strcmp(prop->name, "ANNIVERSARY") == 0) {
            return INV_DT;
        }

        if (prop->values->length == 0 || prop->values->head == NULL) {
            return INV_PROP;
        }

        Node* propParamNode = prop->parameters->head;
        while (propParamNode != NULL) {
            Parameter* param = (Parameter*)propParamNode->data;
            if (param == NULL || param->name == NULL || param->value == NULL || 
                strlen(param->name) == 0 || strlen(param->value) == 0) {
                return INV_PROP;
            }
            propParamNode = propParamNode->next;
        }

        if (strcmp(prop->name, "N") == 0) {
            nCount++;

            if (prop->values->length != 5) {
                return INV_PROP;
            }
        } 
        else if (strcmp(prop->name, "KIND") == 0) {
            kindCount++;
        }
        else if (strcmp(prop->name, "GENDER") == 0) {
            genderCount++;
        }
        else if (strcmp(prop->name, "ORG") == 0) {
            orgCount++;
        }
        else if (strcmp(prop->name, "CATEGORIES") == 0) {
            categoriesCount++;
        }
        else if (strcmp(prop->name, "PRODID") == 0) {
            prodidCount++;
        }
        else if (strcmp(prop->name, "REV") == 0) {
            revCount++;
        }
        else if (strcmp(prop->name, "UID") == 0) {
            uidCount++;
        }
        else if (strcmp(prop->name, "ADR") == 0) {

            if (prop->values->length != 7) {
                return INV_PROP;
            }
        }
        else if (strcmp(prop->name, "GEO") == 0) {

            if (prop->values->length != 2) {
                return INV_PROP;
            }
        }

        const char* allowedProps[] = {
            "SOURCE", "KIND", "FN", "N", "NICKNAME", "PHOTO", "BDAY", "ANNIVERSARY", 
            "GENDER", "ADR", "TEL", "EMAIL", "IMPP", "LANG", "TZ", "GEO", "TITLE", 
            "ROLE", "LOGO", "ORG", "MEMBER", "RELATED", "CATEGORIES", "NOTE", "PRODID", 
            "REV", "SOUND", "UID", "CLIENTPIDMAP", "URL", "KEY", "FBURL", "CALADRURI", 
            "CALURI", "XML"
        };

        bool isAllowedProp = false;
        for (int i = 0; i < sizeof(allowedProps) / sizeof(allowedProps[0]); i++) {
            if (strcmp(prop->name, allowedProps[i]) == 0) {
                isAllowedProp = true;
                break;
            }
        }

        if (!isAllowedProp) {
            return INV_PROP;
        }

        propNode = propNode->next;
    }

    int counts[] = {kindCount, nCount, genderCount, orgCount, categoriesCount, prodidCount, revCount, uidCount};
    int numCounts = sizeof(counts) / sizeof(counts[0]);

    for (int i = 0; i < numCounts; i++) {
        if (counts[i] > 1) {
            return INV_PROP;
        }
    }

    return OK;
}

void deleteValue(void* toBeDeleted) {
    if (toBeDeleted == NULL) {
        return;
    }

    char* value = (char*)toBeDeleted;
    free(value);
}

int compareValues(const void* first, const void* second) {
    if (first == NULL || second == NULL) {
        return 0;
    }

    char* firstValue = (char*)first;
    char* secondValue = (char*)second;

    return strcmp(firstValue, secondValue);
}

char* valueToString(void* val) {
    if (val == NULL) {
        return strdup("NULL");
    }

    char* value = (char*)val;
    return strdup(value);
}

void deleteParameter(void* toBeDeleted) {
    if (toBeDeleted == NULL) {
        return;
    }

    Parameter* param = (Parameter*)toBeDeleted;
    free(param->name);
    free(param->value);
    free(param);
}

int compareParameters(const void* first, const void* second) {
    if (first == NULL || second == NULL) {
        return 0;
    }

    Parameter* firstParam = (Parameter*)first;
    Parameter* secondParam = (Parameter*)second;

    int nameResult = strcmp(firstParam->name, secondParam->name);
    if (nameResult != 0) {
        return nameResult;
    }

    return strcmp(firstParam->value, secondParam->value);
}

char* parameterToString(void* param) {
    if (param == NULL) {
        return strdup("NULL");
    }

    Parameter* parameter = (Parameter*)param;

    char* str = malloc(strlen(parameter->name) + strlen(parameter->value) + 10);
    if (str == NULL) {
        return strdup("Memory allocation failed");
    }

    sprintf(str, "%s=%s", parameter->name, parameter->value);
    return str;
}

VCardErrorCode createCard(char* fileName, Card** newCardObject) {

    if (fileName == NULL || fileName[0] == '\0') {
        *newCardObject = NULL;
        return INV_FILE;
    }

    char* dot = strrchr(fileName, '.');
    if (!dot || (strcmp(dot, ".vcf") != 0 && strcmp(dot, ".vcard") != 0)) {
        *newCardObject = NULL;
        return INV_FILE;
    }

    FILE* file = fopen(fileName, "rb");  
    if (!file) {
        *newCardObject = NULL;
        return INV_FILE;
    }

    char buffer[1000];
    size_t bytesRead = 0;
    bool hasCRLF = false;
    bool hasLF = false;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytesRead - 1; i++) {
            if (buffer[i] == '\r' && buffer[i+1] == '\n') {
                hasCRLF = true;
            } else if (buffer[i] != '\r' && buffer[i+1] == '\n') {
                hasLF = true;
            }
        }
    }

    if (hasLF && !hasCRLF) {
        fclose(file);
        *newCardObject = NULL;
        return INV_PROP;
    }

    fseek(file, 0, SEEK_SET);

    *newCardObject = malloc(sizeof(Card));
    if (!*newCardObject) {
        fclose(file);
        return OTHER_ERROR;
    }

    (*newCardObject)->fn = NULL;
    (*newCardObject)->birthday = NULL;
    (*newCardObject)->anniversary = NULL;
    (*newCardObject)->optionalProperties = initializeList(propertyToString, deleteProperty, compareProperties);

    char readBuffer[1000] = "";  
    char line[1000];         
    bool foundBegin = false;
    bool foundFN = false;
    bool foundEnd = false;
    bool foundVersion = false;

    while (fgets(line, sizeof(line), file)) {

        line[strcspn(line, "\r\n")] = 0;

        if ((line[0] == ' ' || line[0] == '\t') && strlen(readBuffer) > 0) {

            strcat(readBuffer, line + 1);
        } else {

            if (strlen(readBuffer) > 0) {

                if (strcmp(readBuffer, "BEGIN:VCARD") == 0) {
                    foundBegin = true;
                }
                else if (strncmp(readBuffer, "VERSION:", 8) == 0 && foundBegin) {

                    if (strcmp(readBuffer + 8, "4.0") == 0) {
                        foundVersion = true;
                    }
                }
                else if (strncmp(readBuffer, "FN:", 3) == 0 && foundBegin) {

                    if (strlen(readBuffer) <= 3) {
                        deleteCard(*newCardObject);
                        fclose(file);
                        return INV_PROP;
                    }

                    Property* fnProp = malloc(sizeof(Property));
                    if (!fnProp) {
                        deleteCard(*newCardObject);
                        fclose(file);
                        return OTHER_ERROR;
                    }

                    fnProp->name = strdup("FN");
                    fnProp->group = strdup("");
                    fnProp->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
                    fnProp->values = initializeList(valueToString, deleteValue, compareValues);

                    insertBack(fnProp->values, strdup(readBuffer + 3));

                    (*newCardObject)->fn = fnProp;
                    foundFN = true;
                }
                else if (strncmp(readBuffer, "BDAY", 4) == 0 && foundBegin) {

                    char* colonPos = strchr(readBuffer, ':');
                    if (!colonPos || strlen(colonPos) <= 1) {

                        deleteCard(*newCardObject);
                        fclose(file);
                        return INV_PROP;
                    }

                    DateTime* birthday = malloc(sizeof(DateTime));
                    if (!birthday) {
                        deleteCard(*newCardObject);
                        fclose(file);
                        return OTHER_ERROR;
                    }

                    birthday->isText = false;
                    birthday->UTC = false;
                    birthday->date = strdup("");
                    birthday->time = strdup("");
                    birthday->text = strdup("");

                    bool isTextValue = false;
                    char* valuePos = strstr(readBuffer, ";VALUE=text");
                    if (valuePos && valuePos < colonPos) {
                        isTextValue = true;
                        birthday->isText = true;
                    }

                    char* value = colonPos + 1;

                    size_t valueLen = strlen(value);
                    if (valueLen > 0 && value[valueLen - 1] == 'Z') {
                        birthday->UTC = true;
                        value[valueLen - 1] = '\0'; 
                    }

                    if (isTextValue) {

                        free(birthday->text);
                        birthday->text = strdup(value);
                    } else if (strncmp(value, "--", 2) == 0) {

                        free(birthday->date);
                        birthday->date = strdup(value);
                    } else if (value[0] == '-') {

                        birthday->isText = true;
                        free(birthday->text);
                        birthday->text = strdup(value);
                    } else {

                        char* timePos = strchr(value, 'T');
                        if (timePos) {

                            *timePos = '\0';
                            free(birthday->date);
                            birthday->date = strdup(value);
                            free(birthday->time);
                            birthday->time = strdup(timePos + 1);
                        } else {

                            free(birthday->date);
                            birthday->date = strdup(value);
                        }
                    }

                    (*newCardObject)->birthday = birthday;
                }
                else if (strncmp(readBuffer, "ANNIVERSARY", 11) == 0 && foundBegin) {

                    char* colonPos = strchr(readBuffer, ':');
                    if (!colonPos || strlen(colonPos) <= 1) {

                        deleteCard(*newCardObject);
                        fclose(file);
                        return INV_PROP;
                    }

                    DateTime* anniversary = malloc(sizeof(DateTime));
                    if (!anniversary) {
                        deleteCard(*newCardObject);
                        fclose(file);
                        return OTHER_ERROR;
                    }

                    anniversary->isText = false;
                    anniversary->UTC = false;
                    anniversary->date = strdup("");
                    anniversary->time = strdup("");
                    anniversary->text = strdup("");

                    bool isTextValue = false;
                    char* valuePos = strstr(readBuffer, ";VALUE=text");
                    if (valuePos && valuePos < colonPos) {
                        isTextValue = true;
                        anniversary->isText = true;
                    }

                    char* value = colonPos + 1;

                    size_t valueLen = strlen(value);
                    if (valueLen > 0 && value[valueLen - 1] == 'Z') {
                        anniversary->UTC = true;
                        value[valueLen - 1] = '\0'; 
                    }

                    if (isTextValue) {

                        free(anniversary->text);
                        anniversary->text = strdup(value);
                    } else if (strncmp(value, "--", 2) == 0) {

                        free(anniversary->date);
                        anniversary->date = strdup(value);
                    } else if (value[0] == '-') {

                        anniversary->isText = true;
                        free(anniversary->text);
                        anniversary->text = strdup(value);
                    } else {

                        char* timePos = strchr(value, 'T');
                        if (timePos) {

                            *timePos = '\0';
                            free(anniversary->date);
                            anniversary->date = strdup(value);
                            free(anniversary->time);
                            anniversary->time = strdup(timePos + 1);
                        } else {

                            free(anniversary->date);
                            anniversary->date = strdup(value);
                        }
                    }

                    (*newCardObject)->anniversary = anniversary;
                }
                else if (strchr(readBuffer, ':') != NULL && foundBegin && 
                         strcmp(readBuffer, "END:VCARD") != 0) {

                    char* colonPos = strchr(readBuffer, ':');

                    if (colonPos == NULL || strlen(colonPos) <= 1) {
                        deleteCard(*newCardObject);
                        fclose(file);
                        return INV_PROP;
                    }

                    if (colonPos == readBuffer) {
                        deleteCard(*newCardObject);
                        fclose(file);
                        return INV_PROP;
                    }

                    if (strncmp(readBuffer, "VERSION:", 8) == 0 || 
                        (strncmp(readBuffer, "FN:", 3) == 0 && foundFN) || 
                        (strncmp(readBuffer, "BDAY", 4) == 0 && (*newCardObject)->birthday != NULL) ||
                        (strncmp(readBuffer, "ANNIVERSARY", 11) == 0 && (*newCardObject)->anniversary != NULL)) {

                        strcpy(readBuffer, line);
                        continue;
                    }

                    Property* optProp = malloc(sizeof(Property));
                    if (!optProp) {
                        deleteCard(*newCardObject);
                        fclose(file);
                        return OTHER_ERROR;
                    }

                    *colonPos = '\0';

                    char propName[100] = "";
                    char* dotPos = strchr(readBuffer, '.');
                    if (dotPos) {
                        *dotPos = '\0';
                        optProp->group = strdup(readBuffer);
                        strcpy(propName, dotPos + 1);
                    } else {
                        optProp->group = strdup("");
                        strcpy(propName, readBuffer);
                    }

                    char* paramPos = strchr(propName, ';');
                    if (paramPos) {
                        *paramPos = '\0';
                        optProp->name = strdup(propName);

                        optProp->parameters = initializeList(parameterToString, deleteParameter, compareParameters);

                        char* param = paramPos + 1;
                        char* nextParam;

                        while (param < colonPos) {
                            Parameter* newParam = malloc(sizeof(Parameter));
                            if (!newParam) {
                                free(optProp->name);
                                free(optProp->group);
                                free(optProp);
                                deleteCard(*newCardObject);
                                fclose(file);
                                return OTHER_ERROR;
                            }

                            nextParam = strchr(param, ';');
                            if (nextParam) {
                                *nextParam = '\0';
                            }

                            char* paramValuePos = strchr(param, '=');
                            if (paramValuePos) {
                                *paramValuePos = '\0';
                                newParam->name = strdup(param);
                                newParam->value = strdup(paramValuePos + 1);
                            } else {

                                free(newParam);
                                free(optProp->name);
                                free(optProp->group);
                                freeList(optProp->parameters);
                                free(optProp);
                                deleteCard(*newCardObject);
                                fclose(file);
                                return INV_PROP;
                            }

                            insertBack(optProp->parameters, newParam);

                            if (!nextParam) break;
                            param = nextParam + 1;
                            if (param >= colonPos) break;
                        }
                    } else {
                        optProp->name = strdup(propName);
                        optProp->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
                    }

                    optProp->values = initializeList(valueToString, deleteValue, compareValues);

                    char* valueStr = strdup(colonPos + 1);

                    if (strcmp(optProp->name, "N") == 0) {

                        char* valueCopy = strdup(valueStr);
                        char* current = valueCopy;
                        int components = 0;

                        for (int i = 0; i < 5; i++) {
                            char* semicolon = strchr(current, ';');
                            if (semicolon) {
                                *semicolon = '\0';

                                insertBack(optProp->values, strdup(current));
                                current = semicolon + 1;
                                components++;
                            } else {

                                insertBack(optProp->values, strdup(current));
                                components++;
                                break;
                            }
                        }

                        while (components < 5) {
                            insertBack(optProp->values, strdup(""));
                            components++;
                        }

                        free(valueCopy);
                    } 
                    else if (strcmp(optProp->name, "ADR") == 0) {

                        char* valueCopy = strdup(valueStr);
                        char* current = valueCopy;
                        int components = 0;

                        for (int i = 0; i < 7; i++) {
                            char* semicolon = strchr(current, ';');
                            if (semicolon) {
                                *semicolon = '\0';

                                insertBack(optProp->values, strdup(current));
                                current = semicolon + 1;
                                components++;
                            } else {

                                insertBack(optProp->values, strdup(current));
                                components++;
                                break;
                            }
                        }

                        while (components < 7) {
                            insertBack(optProp->values, strdup(""));
                            components++;
                        }

                        free(valueCopy);
                    } 
                    else if (strcmp(optProp->name, "GEO") == 0) {

                        char* valueCopy = strdup(valueStr);

                        char* geoPrefix = strstr(valueCopy, "geo:");
                        char* valueToProcess = valueCopy;

                        if (geoPrefix) {

                            valueToProcess = geoPrefix + 4;
                        }

                        char* comma = strchr(valueToProcess, ',');
                        if (comma) {
                            *comma = '\0';
                            insertBack(optProp->values, strdup(valueToProcess));
                            insertBack(optProp->values, strdup(comma + 1));
                        } else {

                            insertBack(optProp->values, strdup(valueToProcess));

                            insertBack(optProp->values, strdup(""));
                        }

                        free(valueCopy);
                    }
                    else if (strchr(valueStr, ';') != NULL) {

                        char* valueCopy = strdup(valueStr);
                        char* current = valueCopy;
                        int components = 0;

                        int semicolonCount = 0;
                        for (char* c = valueCopy; *c; c++) {
                            if (*c == ';') semicolonCount++;
                        }

                        for (int i = 0; i <= semicolonCount; i++) {
                            char* semicolon = strchr(current, ';');
                            if (semicolon) {
                                *semicolon = '\0';

                                insertBack(optProp->values, strdup(current));
                                current = semicolon + 1;
                                components++;
                            } else {

                                insertBack(optProp->values, strdup(current));
                                components++;
                                break;
                            }
                        }

                        free(valueCopy);
                    } else {

                        if (strlen(valueStr) > 0) {
                            insertBack(optProp->values, strdup(valueStr));
                        } else {

                            free(valueStr);
                            free(optProp->name);
                            free(optProp->group);
                            freeList(optProp->parameters);
                            freeList(optProp->values);
                            free(optProp);
                            deleteCard(*newCardObject);
                            fclose(file);
                            return INV_PROP;
                        }
                    }

                    free(valueStr);

                    insertBack((*newCardObject)->optionalProperties, optProp);
                }
                else if (strcmp(readBuffer, "END:VCARD") == 0) {
                    foundEnd = true;
                }
            }

            strcpy(readBuffer, line);
        }
    }

    if (strlen(readBuffer) > 0) {
        if (strcmp(readBuffer, "END:VCARD") == 0) {
            foundEnd = true;
        }
    }

    fclose(file);

    if (!foundBegin || !foundVersion || !foundFN || !foundEnd) {
        deleteCard(*newCardObject);
        *newCardObject = NULL;
        return INV_CARD;
    }

    return OK;
}

char* cardToString(const Card* obj) {
    if (obj == NULL || obj->fn == NULL) {
        return strdup("null");
    }

    char* str = malloc(16384 * sizeof(char));
    if (!str) {
        return strdup("Memory allocation failed");
    }

    str[0] = '\0';

    strcat(str, "===== vCard Information =====\n");

    strcat(str, "\n== Full Name (FN) ==\n");
    strcat(str, "Property Name: FN\n");

    strcat(str, "Group: ");
    strcat(str, obj->fn->group && obj->fn->group[0] != '\0' ? obj->fn->group : "None");
    strcat(str, "\n");

    strcat(str, "Parameters:\n");
    if (!obj->fn->parameters || obj->fn->parameters->head == NULL) {
        strcat(str, "  * No parameters\n");
    } else {
        Node* paramNode = obj->fn->parameters->head;
        while (paramNode) {
            Parameter* param = (Parameter*)paramNode->data;
            char paramStr[1024];
            snprintf(paramStr, sizeof(paramStr), "  * %s: %s\n", 
                     param->name ? param->name : "Unknown", 
                     param->value ? param->value : "");
            strcat(str, paramStr);
            paramNode = paramNode->next;
        }
    }

    strcat(str, "Values:\n");
    if (obj->fn->values && obj->fn->values->head) {
        Node* valueNode = obj->fn->values->head;
        while (valueNode) {
            char* value = (char*)valueNode->data;
            char valueStr[1024];
            snprintf(valueStr, sizeof(valueStr), "  * %s\n", value ? value : "Empty");
            strcat(str, valueStr);
            valueNode = valueNode->next;
        }
    } else {
        strcat(str, "  * No values\n");
    }

    if (obj->birthday) {
        strcat(str, "\n== BDAY Property ==\n");
        char* birthdayStr = dateToString(obj->birthday);
        strcat(str, birthdayStr);
        free(birthdayStr);
    }

    if (obj->anniversary) {
        strcat(str, "\n== ANNIVERSARY Property ==\n");
        char* anniversaryStr = dateToString(obj->anniversary);
        strcat(str, anniversaryStr);
        free(anniversaryStr);
    }

    strcat(str, "\n== Optional Properties ==\n");
    if (!obj->optionalProperties || obj->optionalProperties->head == NULL) {
        strcat(str, "  * No optional properties\n");
    } else {
        Node* propNode = obj->optionalProperties->head;
        while (propNode) {
            Property* prop = (Property*)propNode->data;
            char propStr[4096];

            snprintf(propStr, sizeof(propStr), "== %s Property ==\n", prop->name);
            strcat(str, propStr);

            snprintf(propStr, sizeof(propStr), "Group: %s\n", 
                     prop->group && prop->group[0] != '\0' ? prop->group : "None");
            strcat(str, propStr);

            strcat(str, "Parameters:\n");
            if (!prop->parameters || prop->parameters->head == NULL) {
                strcat(str, "  * No parameters\n");
            } else {
                Node* paramNode = prop->parameters->head;
                while (paramNode) {
                    Parameter* param = (Parameter*)paramNode->data;
                    snprintf(propStr, sizeof(propStr), "  * %s: %s\n", 
                             param->name ? param->name : "Unknown", 
                             param->value ? param->value : "");
                    strcat(str, propStr);
                    paramNode = paramNode->next;
                }
            }

            strcat(str, "Values:\n");
            if (!prop->values || prop->values->head == NULL) {
                strcat(str, "  * No values\n");
            } else {
                if (strcmp(prop->name, "N") == 0) {

                    Node* valueNode = prop->values->head;
                    int valueIndex = 0;
                    const char* valueLabels[] = {
                        "Family Name", "Given Name", "Additional Names", 
                        "Honorific Prefixes", "Honorific Suffixes"
                    };

                    while (valueNode && valueIndex < 5) {
                        char* value = (char*)valueNode->data;

                        snprintf(propStr, sizeof(propStr), "  * %s: %s\n", 
                                valueLabels[valueIndex], 
                                (value && strlen(value) > 0) ? value : "");
                        strcat(str, propStr);

                        valueNode = valueNode->next;
                        valueIndex++;
                    }

                    while (valueIndex < 5) {
                        snprintf(propStr, sizeof(propStr), "  * %s: \n", valueLabels[valueIndex]);
                        strcat(str, propStr);
                        valueIndex++;
                    }
                } 
                else if (strcmp(prop->name, "ADR") == 0) {

                    Node* valueNode = prop->values->head;
                    int valueIndex = 0;
                    const char* valueLabels[] = {
                        "Post Office Box", "Extended Address", "Street", 
                        "Locality", "Region", "Postal Code", "Country"
                    };

                    while (valueNode && valueIndex < 7) {
                        char* value = (char*)valueNode->data;

                        snprintf(propStr, sizeof(propStr), "  * %s: %s\n", 
                                valueLabels[valueIndex], 
                                (value && strlen(value) > 0) ? value : "");
                        strcat(str, propStr);

                        valueNode = valueNode->next;
                        valueIndex++;
                    }

                    while (valueIndex < 7) {
                        snprintf(propStr, sizeof(propStr), "  * %s: \n", valueLabels[valueIndex]);
                        strcat(str, propStr);
                        valueIndex++;
                    }
                }
                else if (strcmp(prop->name, "GEO") == 0) {

                    Node* valueNode = prop->values->head;
                    int valueIndex = 0;
                    const char* valueLabels[] = {"Latitude", "Longitude"};

                    while (valueNode && valueIndex < 2) {
                        char* value = (char*)valueNode->data;

                        snprintf(propStr, sizeof(propStr), "  * %s: %s\n", 
                                valueLabels[valueIndex], 
                                (value && strlen(value) > 0) ? value : "");
                        strcat(str, propStr);

                        valueNode = valueNode->next;
                        valueIndex++;
                    }

                    while (valueIndex < 2) {
                        snprintf(propStr, sizeof(propStr), "  * %s: \n", valueLabels[valueIndex]);
                        strcat(str, propStr);
                        valueIndex++;
                    }
                }
                else {

                    Node* valueNode = prop->values->head;
                    while (valueNode) {
                        char* value = (char*)valueNode->data;
                        snprintf(propStr, sizeof(propStr), "  * %s\n", 
                                (value && strlen(value) > 0) ? value : "Empty");
                        strcat(str, propStr);
                        valueNode = valueNode->next;
                    }
                }
            }

            propNode = propNode->next;
        }
    }

    strcat(str, "===== End of vCard =====\n");

    return str;
}

void deleteCard(Card* obj) {

    if (obj == NULL) {
        return;
    }

    if (obj->fn != NULL) {
        deleteProperty(obj->fn);
    }

    if (obj->optionalProperties != NULL) {
        freeList(obj->optionalProperties);
    }

    if (obj->birthday != NULL) {
        deleteDate(obj->birthday);
    }

    if (obj->anniversary != NULL) {
        deleteDate(obj->anniversary);
    }

    free(obj);
}

void deleteProperty(void* toBeDeleted) {
    if (toBeDeleted == NULL) {
        return;
    }

    Property* prop = (Property*)toBeDeleted;

    free(prop->name);
    free(prop->group);

    if (prop->parameters != NULL) {
        freeList(prop->parameters);
    }

    if (prop->values != NULL) {
        freeList(prop->values);
    }

    free(prop);
}

int compareProperties(const void* first, const void* second) {
    if (first == NULL || second == NULL) {
        return 0;
    }

    Property* firstProp = (Property*)first;
    Property* secondProp = (Property*)second;

    int nameResult = strcmp(firstProp->name, secondProp->name);
    if (nameResult != 0) {
        return nameResult;
    }

    return strcmp(firstProp->group, secondProp->group);
}

char* propertyToString(void* prop) {
    if (prop == NULL) {
        return strdup("NULL Property");
    }

    Property* property = (Property*)prop;

    size_t bufferSize = strlen(property->name) + 20;
    char* str = malloc(bufferSize);
    if (str == NULL) {
        return strdup("Memory allocation failed");
    }

    if (property->group && property->group[0] != '\0') {
        snprintf(str, bufferSize, "%s.%s", property->group, property->name);
    } else {
        snprintf(str, bufferSize, "%s", property->name);
    }

    return str;
}

char* errorToString(VCardErrorCode err) {
    switch (err) {
        case OK:
            return strdup("OK");
        case INV_FILE:
            return strdup("Invalid file");
        case INV_CARD:
            return strdup("Invalid card format");
        case INV_PROP:
            return strdup("Invalid property");
        case INV_DT:
            return strdup("Invalid date/time");
        case WRITE_ERROR:
            return strdup("Write error");
        case OTHER_ERROR:
            return strdup("Other error");
        default:
            return strdup("Invalid error code");
    }
}

void deleteDate(void* toBeDeleted) {
    DateTime* dt = (DateTime*)toBeDeleted;
    if (dt) {
        if (dt->date) free(dt->date);
        if (dt->time) free(dt->time);
        if (dt->text) free(dt->text);
        free(dt);
    }
}

int compareDates(const void* first, const void* second) {

    return 0;
}

char* dateToString(void* date) {
    DateTime* dt = (DateTime*)date;
    if (!dt) {
        return strdup("NULL DateTime");
    }

    char* str = malloc(256 * sizeof(char));
    if (!str) {
        return strdup("Memory allocation failed");
    }

    str[0] = '\0'; 

    strcat(str, "Type: ");
    strcat(str, dt->isText ? "Text" : "Date-Time");
    strcat(str, "\n");

    strcat(str, "UTC: ");
    strcat(str, dt->UTC ? "true" : "false");
    strcat(str, "\n");

    if (dt->isText) {

        strcat(str, "Text Value: ");
        strcat(str, dt->text ? dt->text : "");
        strcat(str, "\n");
    } else {

        if (dt->date && strlen(dt->date) > 0) {
            strcat(str, "Date: ");
            strcat(str, dt->date);
            strcat(str, "\n");
        }

        if (dt->time && strlen(dt->time) > 0) {
            strcat(str, "Time: ");
            strcat(str, dt->time);
            strcat(str, "\n");
        }
    }

    return str;
}