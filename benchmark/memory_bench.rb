# frozen_string_literal: true

require 'bundler/setup'
require 'csv'
require 'zsv'
require 'tempfile'

# Memory profiling
def measure_memory
  GC.start
  before = `ps -o rss= -p #{Process.pid}`.to_i
  yield
  GC.start
  after = `ps -o rss= -p #{Process.pid}`.to_i
  (after - before) / 1024.0 # MB
end

# Create large test file
def create_large_file(rows: 100_000)
  file = Tempfile.new(['memory_bench', '.csv'])

  file.puts('id,name,email,age,city')
  rows.times do |i|
    file.puts("#{i},User#{i},user#{i}@example.com,#{20 + (i % 50)},City#{i % 100}")
  end

  file.close
  file # Return tempfile object to prevent GC
end

puts 'Creating test data (100K rows)...'
tempfile = create_large_file
test_file = tempfile.path

puts "\n=== Memory Usage Comparison ==="

csv_memory = measure_memory do
  count = 0
  CSV.foreach(test_file) { |_row| count += 1 }
  puts "CSV processed #{count} rows"
end

zsv_memory = measure_memory do
  count = 0
  ZSV.foreach(test_file) { |_row| count += 1 }
  puts "ZSV processed #{count} rows"
end

puts "\nMemory used (MB):"
puts "  CSV: #{csv_memory.round(2)} MB"
puts "  ZSV: #{zsv_memory.round(2)} MB"
difference = (csv_memory - zsv_memory).round(2)
reduction = ((1 - (zsv_memory / csv_memory)) * 100).round(1)
puts "  Difference: #{difference} MB (#{reduction}% reduction)"

tempfile.unlink
