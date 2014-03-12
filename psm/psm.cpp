#include <cpplib/logger.h>
#include "psmcontext.h"

static Logger g_logger;

int main(int argc, char **argv)
{
  PSMContext psmctx("psm.config", g_logger);
  psmctx.Start();

  //
  return 0;
}
