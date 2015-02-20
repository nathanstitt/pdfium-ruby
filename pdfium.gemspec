# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'pdfium/version'

Gem::Specification.new do |spec|
  spec.name          = "pdfium"
  spec.version       = Pdfium::VERSION
  spec.authors       = ["Nathan Stitt"]
  spec.email         = ["nathan@stitt.org"]
  spec.summary       = %q{PDFium test}
  spec.description   = %q{PDFium test gem, nothing much to say atm}
  spec.homepage      = ""
  spec.license       = "MIT"

  spec.files         = `git ls-files -z`.split("\x0")
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.7"
  spec.add_development_dependency "rake", "~> 10.0"

  spec.add_development_dependency "guard-minitest"
  spec.add_development_dependency "guard-rake"
  spec.add_development_dependency "rake-compiler"
end
