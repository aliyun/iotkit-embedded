#ifndef DM_THING_H
#define DM_THING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "interface/thing_abstract.h"
#include "interface/log_abstract.h"
#include "dsl.h"

#define DEFAULT_DSL_DELIMITER '.'
#define MAX_IDENTIFIER_LENGTH (50+1) /* user input max id lengh is 50 characters. */

#define DM_THING_CLASS get_dm_thing_class()

void property_iterator(void* _self, handle_item_t handle_fp, ...);
void event_iterator(void* _self, handle_item_t handle_fp, ...);
void service_iterator(void* _self, handle_item_t handle_fp, ...);

typedef struct {
    const void*    _;
    char*          _name; /* dm thing object name. */
    char*          _dsl_string; /* dsl string from yun or customer input. */
    size_t         _dsl_string_length;
    void*          _json_object; /* json object after dsl string parsed. */
    dsl_template_t dsl_template;
    int            _arr_index;
} dm_thing_t;

extern const void* get_dm_thing_class();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DM_THING_H */
