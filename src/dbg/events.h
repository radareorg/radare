#ifndef EVENT_H
#define EVENT_H

void event_ignore_list();
int event_set_ignored(char *name, int ignored);
int event_is_ignored(int id);

#endif
