#include <granny.h>
#include <cstdio>
int main()
{
	for (int i = -1; i <= 5; ++i) printf("type %d: \"%s\"\n", i, GrannyGetLogMessageTypeString((granny_log_message_type) i));
	for (int i = -1; i <= GrannyOnePastLastMessageOrigin; ++i) printf("origin %d: \"%s\"\n", i, GrannyGetLogMessageOriginString((granny_log_message_origin) i));
	return 0;
}
