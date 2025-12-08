#!/usr/bin/env ruby
# frozen_string_literal: true

require 'bundler/setup'
require 'zsv'

# Create sample CSV file
sample_csv = <<~CSV
  id,name,email,age
  1,Alice,alice@example.com,30
  2,Bob,bob@example.com,25
  3,Charlie,charlie@example.com,35
CSV

File.write('sample.csv', sample_csv)

puts '=== Example 1: Parse String ==='
rows = ZSV.parse(sample_csv)
puts rows.inspect

puts "\n=== Example 2: Parse with Headers ==="
rows = ZSV.parse(sample_csv, headers: true)
rows.each do |row|
  puts "#{row['name']} (#{row['age']}) - #{row['email']}"
end

puts "\n=== Example 3: Stream File ==="
ZSV.foreach('sample.csv', headers: true) do |row|
  puts "Processing user: #{row['name']}"
end

puts "\n=== Example 4: Parser Instance ==="
parser = ZSV.open('sample.csv', headers: true)
puts "First row: #{parser.shift.inspect}"
puts "Second row: #{parser.shift.inspect}"
parser.rewind
puts "After rewind: #{parser.shift.inspect}"
parser.close

puts "\n=== Example 5: Custom Delimiter ==="
tsv_data = "a\tb\tc\n1\t2\t3\n"
rows = ZSV.parse(tsv_data, col_sep: "\t")
puts rows.inspect

puts "\n=== Example 6: Block Form ==="
ZSV.open('sample.csv') do |parser|
  count = 0
  parser.each { count += 1 }
  puts "Total rows: #{count}"
end

# Cleanup
File.unlink('sample.csv')

puts "\nAll examples completed successfully!"
