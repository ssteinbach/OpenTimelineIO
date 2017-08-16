
#define OTIO_INTERNAL

#include "OpenTimelineIO.h"


#define FALSE 0
#define TRUE !FALSE

typedef unsigned int uint;

typedef struct{
	void *first;
	void *last;
	uint count;
}OTIOList;

typedef struct{
	char *name;
	void *parent;
	void *meta;
	void *next;
	void *prev;
	OTIOObjectType type;
	void *metadata;
	void *user_data;
}OTIOHeader;

typedef struct{
	OTIOHeader header;
	OTIOList children;
	OTIOList markers;
	OTIOList effects;
	OTIO_RationalTime start_time;
	OTIO_RationalTime duration_time;
}OTIOComposable;

typedef struct{
	OTIOHeader header;
	char *kind;
	char *uri;
	OTIO_RationalTime available_range;
}OTIOMediaReferences;

typedef struct{
	OTIOComposable header;
	char *transition_type;
	OTIO_RationalTime in_time;
	OTIO_RationalTime out_time;
}OTIOTransition;

typedef struct{
	OTIOHeader header;
	OTIO_RationalTime time;
	char *message;
}OTIOMarker;

typedef struct{
	OTIOHeader header;
}OTIOEffect;
