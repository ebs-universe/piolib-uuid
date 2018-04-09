

#include "test_uuid.h"
#include "sys/rand.h"
#include <uuid/uuid.h>
#include <time/sync.h>
#include <ucdm/descriptor.h>
#include <ucdm/ucdm.h>

uuid_t uuidbns;
uuid_t uuidb1;
uuid_t uuidb3;
uuid_t uuidb4;
uuid_t uuidb5;
uuid_t uuidb6;

char uuidsns[UUID_LENGTH_STRING];
char uuids1[UUID_LENGTH_STRING];
char uuids3[UUID_LENGTH_STRING];
char uuids4[UUID_LENGTH_STRING];
char uuids5[UUID_LENGTH_STRING];
char uuids6[UUID_LENGTH_STRING];

static const char * name = "test";

descriptor_custom_t cd_uuidns = {NULL, 10, UUID_LENGTH_STRING, DESCRIPTOR_ACCTYPE_PTR, {&uuidsns}};
descriptor_custom_t cd_uuid1 =  {NULL, 11, UUID_LENGTH_STRING, DESCRIPTOR_ACCTYPE_PTR, {&uuids1}};
descriptor_custom_t cd_uuid3 =  {NULL, 13, UUID_LENGTH_STRING, DESCRIPTOR_ACCTYPE_PTR, {&uuids3}};
descriptor_custom_t cd_uuid4 =  {NULL, 14, UUID_LENGTH_STRING, DESCRIPTOR_ACCTYPE_PTR, {&uuids4}};
descriptor_custom_t cd_uuid5 =  {NULL, 15, UUID_LENGTH_STRING, DESCRIPTOR_ACCTYPE_PTR, {&uuids5}};
descriptor_custom_t cd_uuid6 =  {NULL, 16, UUID_LENGTH_STRING, DESCRIPTOR_ACCTYPE_PTR, {&uuids6}};

static void uuid_generate(uint16_t __attribute__((unused)) addr, uint16_t ver){
    switch(ver){
        case (0x00):
            uuid_sprintf(uuidsns, &uuidbns);
        case (0x01):
            uuid1(&uuidb1);
            uuid_sprintf(uuids1, &uuidb1);
            break;
        case (0x03):
            uuid3(&uuidb3, &uuidbns, (uint8_t *)name, sizeof(name));
            uuid_sprintf(uuids3, &uuidb3);
            break;
        case (0x04):
            uuid4(&uuidb4);
            uuid_sprintf(uuids4, &uuidb4);
            break;
        case (0x05):
            uuid5(&uuidb5, &uuidbns, (uint8_t *)name, sizeof(name));
            uuid_sprintf(uuids5, &uuidb5);
            break;
        case (0x06):
            uuid6(&uuidb6);
            uuid_sprintf(uuids6, &uuidb6);
            break;
        default:
            break;
    };
}

void uuid_test_init(void){
    rand_init();
    uuid_init();
    // Prepoulate UUIDs
    for (uint8_t i=0; i < 7; i++){
        uuid_generate(0, i);
    }

    // Install Handlers
    ucdm_redirect_regw_func(UCDM_UUID_TEST_BASE_ADDRESS, &uuid_generate);
    
    // Install Descriptors
    descriptor_install(&cd_uuidns);
    descriptor_install(&cd_uuid1);
    descriptor_install(&cd_uuid3);
    descriptor_install(&cd_uuid4);
    descriptor_install(&cd_uuid5);
    descriptor_install(&cd_uuid6);
    
    // Request Time Sync from Host
    tm_sync_request_host();    
}                        
