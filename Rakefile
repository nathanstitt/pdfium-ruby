require "bundler/gem_tasks"
require 'rake/testtask'
require 'rdoc/task'

Rake::TestTask.new do |t|
    t.libs << 'test'
    t.pattern = "test/*_spec.rb"
end

RDOC_FILES = FileList["README.md",
                      "lib/pdfium.rb",
                      "lib/pdfium/*.rb",
                      "ext/pdfium_ext/*.cc"
                     ]
Rake::RDocTask.new do |rd|
    rd.main = "README.md"
    rd.options << "--verbose"
    rd.rdoc_files.include(RDOC_FILES)
end


require "bundler/gem_tasks"

require "rake/extensiontask"
Rake::ExtensionTask.new("pdfium_ext") do | ext |
  ext.source_pattern = "*.cc"
end

task :buildtest => :compile do
    Rake::Task["test"].invoke
end

task :console do
  require 'irb'
  require 'irb/completion'
  require 'pdfium'
  ARGV.clear
  IRB.start
end

# valgrind and Ruby
# http://blog.flavorjon.es/2009/06/easily-valgrind-gdb-your-ruby-c.html
# http://blog.evanweaver.com/2008/02/05/valgrind-and-ruby/
namespace :test do
  # partial-loads-ok and undef-value-errors necessary to ignore
  # spurious (and eminently ignorable) warnings from the ruby
  # interpreter
  VALGRIND_BASIC_OPTS = <<-EOS
 --tool=memcheck
 --dsymutil=yes \
 --num-callers=50 --error-limit=no --leak-check=full \
 --partial-loads-ok=yes --undef-value-errors=no
  EOS

  SUPRESS = ""# "--suppressions=./valgrind.supp"
  desc "run test suite under valgrind with basic ruby options"
  task :valgrind => :compile do
    cmdline = "valgrind #{SUPRESS} #{VALGRIND_BASIC_OPTS} ruby rake test"
    puts cmdline
    system cmdline
  end
end
