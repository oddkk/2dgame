#include "world.h"
#include "int.h"
#include "utils.h"

struct entity *allocate_entity(struct world *world) {
	assert(world->num_active_entities + 1 < MAX_ACTIVE_ENTITIES);
	return &world->active_entities[world->num_active_entities++];
}

void deallocate_entity(struct world *world, size_t id) {
	world->active_entities[id] = world->active_entities[--world->num_active_entities];
}
