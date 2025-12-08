# frozen_string_literal: true

require "mkmf"
require "net/http"
require "uri"
require "fileutils"
require "rubygems/package"
require "zlib"

# ZSV version to compile against
ZSV_VERSION = "1.3.0"
ZSV_URL = "https://github.com/liquidaty/zsv/archive/refs/tags/v#{ZSV_VERSION}.tar.gz".freeze
# Use absolute path relative to the original extconf.rb location
EXTCONF_DIR = File.expand_path(__dir__)
VENDOR_DIR = File.join(EXTCONF_DIR, "vendor")
ZSV_DIR = File.join(VENDOR_DIR, "zsv-#{ZSV_VERSION}")

def download_file(url, destination)
  uri = URI.parse(url)
  File.open(destination, "wb") do |file|
    Net::HTTP.start(uri.host, uri.port, use_ssl: uri.scheme == "https") do |http|
      request = Net::HTTP::Get.new(uri)
      http.request(request) do |response|
        case response
        when Net::HTTPRedirection
          # Follow redirect
          download_file(response["location"], destination)
        when Net::HTTPSuccess
          response.read_body do |chunk|
            file.write(chunk)
          end
        else
          abort("Failed to download: #{response.code} #{response.message}")
        end
      end
    end
  end
end

def extract_tar_gz(tarball, destination)
  Gem::Package::TarReader.new(Zlib::GzipReader.open(tarball)) do |tar|
    tar.each do |entry|
      dest_path = File.join(destination, entry.full_name)

      if entry.directory?
        FileUtils.mkdir_p(dest_path)
      elsif entry.file?
        FileUtils.mkdir_p(File.dirname(dest_path))
        File.binwrite(dest_path, entry.read)
        FileUtils.chmod(entry.header.mode, dest_path)
      end
    end
  end
end

def download_and_extract_zsv
  return if File.directory?(ZSV_DIR)

  puts "Downloading zsv #{ZSV_VERSION}..."
  FileUtils.mkdir_p(VENDOR_DIR)

  tarball = File.join(VENDOR_DIR, "zsv.tar.gz")
  download_file(ZSV_URL, tarball)

  puts "Extracting zsv..."
  extract_tar_gz(tarball, VENDOR_DIR)
  FileUtils.rm_f(tarball)

  abort("zsv directory not found after extraction") unless File.directory?(ZSV_DIR)
  puts "zsv #{ZSV_VERSION} downloaded successfully"
end

def build_zsv
  puts "Building zsv library..."

  # Build zsv static library
  Dir.chdir(ZSV_DIR) do
    # Configure zsv
    system("./configure") or abort("Failed to configure zsv") unless File.exist?("config.mk")

    # Build the library
    Dir.chdir("src") do
      system("make", "build") or abort("Failed to build zsv library")
    end
  end

  puts "zsv library built successfully"
end

# Download and build zsv
download_and_extract_zsv
build_zsv

# Determine build directory based on platform
platform_dir = if RUBY_PLATFORM =~ /darwin/
                 "Darwin"
               elsif RUBY_PLATFORM =~ /linux/
                 "Linux"
               else
                 "generic"
               end

# Find the built library
zsv_lib_dir = File.join(ZSV_DIR, "build", platform_dir, "rel", "gcc", "lib")
zsv_lib = File.join(zsv_lib_dir, "libzsv.a")

abort("libzsv.a not found at #{zsv_lib}") unless File.exist?(zsv_lib)

# Add zsv include path
include_dir = File.join(ZSV_DIR, "include")

# Add compiler and linker flags
$INCFLAGS << " -I#{include_dir}"
$CFLAGS << " -std=c99 -Wall -Wextra"
$CFLAGS << " -O3" # Optimization level

# Configure include and lib paths
dir_config("zsv", include_dir, zsv_lib_dir)

# Find zsv header
abort("zsv.h not found in #{include_dir}") unless have_header("zsv.h")

# Link the static library
$LOCAL_LIBS << " #{zsv_lib}"

# Platform-specific adjustments
if RUBY_PLATFORM =~ /darwin/
  $LDFLAGS << " -framework Foundation"
elsif RUBY_PLATFORM =~ /linux/
  $LIBS << " -lpthread -lm"
end

# Create Makefile
create_makefile("zsv/zsv")
