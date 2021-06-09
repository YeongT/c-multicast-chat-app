#include "define.h"
#include "utils.h"

//# define userGroupObject
typedef struct groupFrame
{
    char name[SIZE_OPTION];
    userSocketObject *members[MAX_CLIENT];
} userGroupObject;
