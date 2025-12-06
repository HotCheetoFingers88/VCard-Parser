#ifndef _CARDHELPER_H
#define _CARDHELPER_H

#include "../include/VCParser.h"

VCardErrorCode parseAndValidateCard(const char* fileName);

char* getFirstValue(List* list);

int getOtherPropertiesCount(Card* card);

void updateFirstValue(List* list, const char* newValue);





Card* createEmptyCard();
VCardErrorCode setFN(Card* card, const char* fullName);

#endif

