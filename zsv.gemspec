# frozen_string_literal: true

require_relative 'lib/zsv/version'

Gem::Specification.new do |spec|
  spec.name = 'zsv'
  spec.version = ZSV::VERSION
  spec.authors = ['sebyx07']
  spec.email = ['gore.sebyx@yahoo.com']

  spec.summary = 'SIMD-accelerated CSV parser using zsv'
  spec.description = "A drop-in replacement for Ruby's CSV stdlib that uses zsv " \
                     '(SIMD-accelerated C library) for 5-6x faster CSV parsing'
  spec.homepage = 'https://github.com/sebyx07/zsv-ruby'
  spec.license = 'MIT'
  spec.required_ruby_version = '>= 3.3.0'

  spec.metadata['homepage_uri'] = spec.homepage
  spec.metadata['source_code_uri'] = spec.homepage
  spec.metadata['changelog_uri'] = "#{spec.homepage}/blob/main/CHANGELOG.md"
  spec.metadata['rubygems_mfa_required'] = 'true'

  # Specify which files should be added to the gem when it is released.
  spec.files = Dir[
    'lib/**/*.rb',
    'ext/**/*.{c,h,rb}',
    'LICENSE*',
    'README.md',
    'CHANGELOG.md'
  ]

  spec.bindir = 'exe'
  spec.executables = spec.files.grep(%r{\Aexe/}) { |f| File.basename(f) }
  spec.require_paths = ['lib']
  spec.extensions = ['ext/zsv/extconf.rb']

  # Development dependencies
  spec.add_development_dependency 'benchmark-ips', '~> 2.0'
  spec.add_development_dependency 'rake', '~> 13.0'
  spec.add_development_dependency 'rake-compiler', '~> 1.2'
  spec.add_development_dependency 'rspec', '~> 3.0'
end
