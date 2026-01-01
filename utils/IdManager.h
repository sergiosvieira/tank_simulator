#ifndef IDMANAGER_H
#define IDMANAGER_H

struct IdManager {
  static int next_id() {
    static int id = 0;
    return ++id;
  }
};

#endif  // IDMANAGER_H
