#include "../include/VCHelpers.h"

VCardErrorCode parseAndValidateCard(const char* fileName) {
    Card* card = NULL;
    VCardErrorCode err = createCard((char*)fileName, &card);

    if (err == OK) {
        err = validateCard(card);
    }

    deleteCard(card);
    return err;
}

int getOtherPropertiesCount(Card* card) {
    if (card == NULL || card->optionalProperties == NULL) {
        return 0;
    }

    return getLength(card->optionalProperties);
}

char* getFirstValue(List* list) {
    if (list == NULL) return NULL;

    void* firstValue = getFromFront(list);

    if (firstValue == NULL) return NULL;

    return (char*)firstValue;
}

void updateFirstValue(List* list, const char* newValue) {
    if (list == NULL || list->head == NULL) {
        return;
    }

    void* oldValue = list->head->data;
    free(oldValue);

    list->head->data = strdup(newValue);
}

Card* createEmptyCard() {
    Card* newCard = malloc(sizeof(Card));
    if (!newCard) {
        return NULL;
    }

    newCard->fn = NULL;
    newCard->optionalProperties = initializeList(&propertyToString, &deleteProperty, &compareProperties);
    newCard->birthday = NULL;
    newCard->anniversary = NULL;

    return newCard;
}

VCardErrorCode setFN(Card* card, const char* fullName) {
    if (!card || !fullName || strlen(fullName) == 0) {
        return INV_PROP;
    }

    Property* fnProp = malloc(sizeof(Property));
    if (!fnProp) {
        return OTHER_ERROR;
    }

    fnProp->name = strdup("FN");
    fnProp->group = strdup("");
    fnProp->parameters = initializeList(&parameterToString, &deleteParameter, &compareParameters);
    fnProp->values = initializeList(&valueToString, &deleteValue, &compareValues);

    char* nameValue = strdup(fullName);
    if (!nameValue) {
        deleteProperty(fnProp);
        return OTHER_ERROR;
    }
    insertBack(fnProp->values, nameValue);

    if (card->fn) {
        insertBack(card->optionalProperties, card->fn);
    }

    card->fn = fnProp;
    return OK;
}

