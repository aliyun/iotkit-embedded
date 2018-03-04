#ifndef NEW_H
#define NEW_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void* new_object(const void* _class, ...);
void  delete_object(void* _class);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NEW_H */
