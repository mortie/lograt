project(
  'lograt', 'cpp',
  default_options: ['cpp_std=c++17'],
)

libgtkmm = dependency('gtkmm-4.0')
libpcre2 = dependency('libpcre2-8')

executable(
  'lograt',
  'src/LogView.cc',
  'src/main.cc',
  'src/MainWindow.cc',
  'src/Pattern.cc',
  'src/PatternEditor.cc',
  dependencies: [libgtkmm, libpcre2],
)
