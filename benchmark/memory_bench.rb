# frozen_string_literal: true

require 'bundler/setup'
require 'csv'
require 'zsv'
require 'tempfile'

# Create large test file
def create_large_file(rows: 100_000)
  file = Tempfile.new(['memory_bench', '.csv'])

  file.puts('id,name,email,age,city')
  rows.times do |i|
    file.puts("#{i},User#{i},user#{i}@example.com,#{20 + (i % 50)},City#{i % 100}")
  end

  file.close
  file
end

# Measure memory when holding all rows
def measure_memory_holding_rows(file_path, parser)
  GC.start(full_mark: true, immediate_sweep: true)
  before = `ps -o rss= -p #{Process.pid}`.to_i

  rows = []
  if parser == :csv
    CSV.foreach(file_path) { |row| rows << row }
  else
    ZSV.foreach(file_path) { |row| rows << row }
  end

  GC.start(full_mark: true, immediate_sweep: true)
  after = `ps -o rss= -p #{Process.pid}`.to_i

  [rows.size, (after - before) / 1024.0]
end

# Count string allocations during streaming
def count_string_allocations(file_path, parser)
  GC.start
  GC.disable

  before_count = ObjectSpace.count_objects[:T_STRING]

  if parser == :csv
    CSV.foreach(file_path) { |_| nil }
  else
    ZSV.foreach(file_path) { |_| nil }
  end

  after_count = ObjectSpace.count_objects[:T_STRING]
  GC.enable

  after_count - before_count
end

puts 'Creating test data (100K rows)...'
tempfile = create_large_file
test_file = tempfile.path

puts "\n=== Memory Usage (Holding All Rows) ==="

csv_count, csv_memory = measure_memory_holding_rows(test_file, :csv)
puts "CSV stdlib: #{csv_count} rows, #{csv_memory.round(1)} MB"

# Force cleanup between tests
GC.start(full_mark: true, immediate_sweep: true)
sleep 0.1

zsv_count, zsv_memory = measure_memory_holding_rows(test_file, :zsv)
puts "ZSV:        #{zsv_count} rows, #{zsv_memory.round(1)} MB"

if csv_memory.positive? && zsv_memory.positive?
  savings = ((1 - (zsv_memory / csv_memory)) * 100).round(1)
  puts "\nMemory savings: #{savings}%"
end

puts "\n=== String Allocations (Streaming 10K rows) ==="
tempfile_small = create_large_file(rows: 10_000)
small_file = tempfile_small.path

csv_allocs = count_string_allocations(small_file, :csv)
GC.start
zsv_allocs = count_string_allocations(small_file, :zsv)

puts "CSV stdlib: #{csv_allocs} strings"
puts "ZSV:        #{zsv_allocs} strings"

if csv_allocs.positive? && zsv_allocs.positive?
  reduction = ((1 - (zsv_allocs.to_f / csv_allocs)) * 100).round(1)
  puts "\nAllocation reduction: #{reduction}%"
end

tempfile.unlink
tempfile_small.unlink
