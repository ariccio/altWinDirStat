#pragma once

namespace Update {

enum Command {
  cmdClean,
  cmdExecute,
};

void init();
void clean();
void execute(bool ask);

}
