#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <logger.h>

int main( int argc, char **argv)
{
  Logging::instance()->defaultConfig();
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );
  runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(), std::cerr ) );
  bool wasSuccessful = runner.run( "", false );
  return wasSuccessful ? 0 : -1;
}
