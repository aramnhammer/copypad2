#include "types.h"

void HandleClayErrors(Clay_ErrorData errorData);

void HandleClayErrors(Clay_ErrorData errorData)
{
    printf("%s", errorData.errorText.chars);
}
