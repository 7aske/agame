//
// Created by nik on 11/25/19.
//

#ifndef AGAME_EVENT_H
#define AGAME_EVENT_H
//
// enum ev_type {
// 	EV_DEFAULT
// };

typedef struct event {
	void (* callback)();
} event_t;

#endif //AGAME_EVENT_H
