#include "types.h"
#include <stdio.h>

void HandleClayErrors(Clay_ErrorData errorData);

void HandleClayErrors(Clay_ErrorData errorData)
{
    printf("%s", errorData.errorText.chars);
}
