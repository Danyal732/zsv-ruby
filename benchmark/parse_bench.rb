# frozen_string_literal: true

require 'bundler/setup'
require 'benchmark/ips'
require 'csv'
require 'zsv'
require 'tempfile'

# Create test data
def create_test_file(rows: 10_000, cols: 10)
  file = Tempfile.new(['bench', '.csv'])

  # Write header
  file.puts((1..cols).map { |i| "col#{i}" }.join(','))

  # Write data rows
  rows.times do |i|
    file.puts((1..cols).map { |j| "value_#{i}_#{j}" }.join(','))
  end

  file.close
  file # Return tempfile object to prevent GC
end

puts 'Creating test data...'
small_tempfile = create_test_file(rows: 1_000, cols: 5)
medium_tempfile = create_test_file(rows: 10_000, cols: 10)
large_tempfile = create_test_file(rows: 100_000, cols: 10)
small_file = small_tempfile.path
medium_file = medium_tempfile.path
large_file = large_tempfile.path

puts "\n=== Small file (1K rows, 5 cols) ==="
Benchmark.ips do |x|
  x.config(time: 5, warmup: 2)

  x.report('CSV (stdlib)') do
    CSV.foreach(small_file) { |row| row }
  end

  x.report('ZSV') do
    ZSV.foreach(small_file) { |row| row }
  end

  x.compare!
end

puts "\n=== Medium file (10K rows, 10 cols) ==="
Benchmark.ips do |x|
  x.config(time: 5, warmup: 2)

  x.report('CSV (stdlib)') do
    CSV.foreach(medium_file) { |row| row }
  end

  x.report('ZSV') do
    ZSV.foreach(medium_file) { |row| row }
  end

  x.compare!
end

puts "\n=== Large file (100K rows, 10 cols) ==="
Benchmark.ips do |x|
  x.config(time: 5, warmup: 2)

  x.report('CSV (stdlib)') do
    CSV.foreach(large_file) { |row| row }
  end

  x.report('ZSV') do
    ZSV.foreach(large_file) { |row| row }
  end

  x.compare!
end

puts "\n=== With headers ==="
Benchmark.ips do |x|
  x.config(time: 5, warmup: 2)

  x.report('CSV (stdlib)') do
    CSV.foreach(medium_file, headers: true) { |row| row }
  end

  x.report('ZSV') do
    ZSV.foreach(medium_file, headers: true) { |row| row }
  end

  x.compare!
end

# Cleanup
small_tempfile.unlink
medium_tempfile.unlink
large_tempfile.unlink
