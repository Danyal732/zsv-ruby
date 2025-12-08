#!/usr/bin/env ruby
# frozen_string_literal: true

require 'bundler/setup'
require 'csv'
require 'zsv'
require 'benchmark'

# Create test data
rows = 10_000
cols = 10

csv_data = []
csv_data << (1..cols).map { |i| "col#{i}" }.join(',')
rows.times do |i|
  csv_data << (1..cols).map { |j| "value_#{i}_#{j}" }.join(',')
end

test_file = 'performance_test.csv'
File.write(test_file, csv_data.join("\n"))

puts 'Performance Comparison'
puts '=' * 60
puts "File: #{rows} rows × #{cols} columns"
puts "Size: #{File.size(test_file) / 1024} KB"
puts '=' * 60

n = 10

Benchmark.bm(15) do |x|
  x.report('CSV (stdlib)') do
    n.times do
      count = 0
      CSV.foreach(test_file) { |_row| count += 1 }
    end
  end

  x.report('ZSV') do
    n.times do
      count = 0
      ZSV.foreach(test_file) { |_row| count += 1 }
    end
  end
end

puts "\n=== Memory Usage (approximate) ==="

GC.start
before_csv = `ps -o rss= -p #{Process.pid}`.to_i
CSV.foreach(test_file) { |row| row }
GC.start
after_csv = `ps -o rss= -p #{Process.pid}`.to_i
csv_memory = (after_csv - before_csv) / 1024.0

GC.start
before_zsv = `ps -o rss= -p #{Process.pid}`.to_i
ZSV.foreach(test_file) { |row| row }
GC.start
after_zsv = `ps -o rss= -p #{Process.pid}`.to_i
zsv_memory = (after_zsv - before_zsv) / 1024.0

puts "CSV: ~#{csv_memory.round(2)} MB"
puts "ZSV: ~#{zsv_memory.round(2)} MB"

if csv_memory.positive? && zsv_memory.positive?
  reduction = ((1 - (zsv_memory / csv_memory)) * 100).round(1)
  puts "Reduction: #{reduction}%"
end

# Cleanup
File.unlink(test_file)
