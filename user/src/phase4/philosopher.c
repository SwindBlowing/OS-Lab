#include "philosopher.h"

// TODO: define some sem if you need
int phy_id[PHI_NUM][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}};
int chops_sem_id[PHI_NUM];

void init() {
  // init some sem if you need
  //TODO();
  for (int i = 0; i < PHI_NUM; i++){
	chops_sem_id[i] = sem_open(1);
	//sem_v(chops_sem_id[i]);
  }
}

void philosopher(int id) {
  // implement philosopher, remember to call `eat` and `think`
  while (1) {
    //TODO();
	if (id % 2 == 0) {
		sem_p(chops_sem_id[phy_id[id][0]]); sem_p(chops_sem_id[phy_id[id][1]]);
		eat(id);
		sem_v(chops_sem_id[phy_id[id][0]]); sem_v(chops_sem_id[phy_id[id][1]]);
	}
	else {
		sem_p(chops_sem_id[phy_id[id][1]]); sem_p(chops_sem_id[phy_id[id][0]]);
		eat(id);
		sem_v(chops_sem_id[phy_id[id][1]]); sem_v(chops_sem_id[phy_id[id][0]]);
	}
    think(id);
  }
}
