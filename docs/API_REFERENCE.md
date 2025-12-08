# ZSV API Quick Reference

## Module Methods

### ZSV.parse(string, **options) → Array
Parse CSV string and return all rows.

```ruby
ZSV.parse("a,b\n1,2\n")
# => [["a", "b"], ["1", "2"]]

ZSV.parse("name,age\nAlice,30\n", headers: true)
# => [{"name" => "Alice", "age" => "30"}]
```

### ZSV.foreach(path, **options) { |row| } → nil
Stream rows from file (memory efficient).

```ruby
ZSV.foreach("data.csv") do |row|
  puts row.inspect
end

# Returns Enumerator without block
enum = ZSV.foreach("data.csv")
enum.first # => ["col1", "col2", ...]
```

### ZSV.read(path, **options) → Array
Read entire file into array.

```ruby
rows = ZSV.read("data.csv")
rows = ZSV.read("data.csv", headers: true)
```

### ZSV.open(path, mode="r", **options) → Parser
Open file and return Parser. Auto-closes with block.

```ruby
parser = ZSV.open("data.csv")
parser.each { |row| ... }
parser.close

# Block form (auto-closes)
ZSV.open("data.csv") do |parser|
  parser.each { |row| ... }
end
```

### ZSV.new(source, **options) → Parser
Create parser from path or string.

```ruby
parser = ZSV.new("data.csv")
parser = ZSV.new("a,b\n1,2\n")  # Auto-detects CSV content
```

## Parser Instance Methods

### #shift → Array|Hash|nil
Read next row (pull-based).

```ruby
parser = ZSV.new("data.csv")
row1 = parser.shift  # First row
row2 = parser.shift  # Second row
nil_row = parser.shift  # nil at EOF
```

### #each { |row| } → self
Iterate all rows. Returns Enumerator without block.

```ruby
parser.each do |row|
  puts row.inspect
end

# Or as enumerator
parser.each.with_index { |row, i| ... }
```

### #rewind → nil
Reset parser to beginning (file-based only).

```ruby
parser.rewind
parser.shift  # First row again
```

### #close → nil
Close parser and release resources.

```ruby
parser.close
```

### #headers → Array|nil
Get header row (if headers mode enabled).

```ruby
parser = ZSV.new("data.csv", headers: true)
parser.shift  # Process first row
parser.headers  # => ["col1", "col2", ...]
```

### #closed? → Boolean
Check if parser is closed.

```ruby
parser.closed?  # => false
parser.close
parser.closed?  # => true
```

### #read → Array
Read all remaining rows.

```ruby
parser = ZSV.new("data.csv")
all_rows = parser.read
```

## Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `headers` | Boolean/Array | `false` | Use first row as headers, or provide custom headers |
| `col_sep` | String | `","` | Column delimiter (single char) |
| `quote_char` | String | `"\""` | Quote character (single char) |
| `skip_lines` | Integer | `0` | Lines to skip at start |
| `encoding` | Encoding | `UTF-8` | Source encoding |
| `liberal_parsing` | Boolean | `false` | Handle malformed CSV |
| `buffer_size` | Integer | `262144` | Buffer size (bytes) |

### Examples

```ruby
# Tab-separated
ZSV.parse("a\tb\n1\t2\n", col_sep: "\t")

# Pipe-separated
ZSV.parse("a|b\n1|2\n", col_sep: "|")

# Skip header comments
ZSV.foreach("data.csv", skip_lines: 2)

# Custom headers
ZSV.parse("1,2\n3,4\n", headers: ["id", "value"])

# Large buffer
ZSV.foreach("huge.csv", buffer_size: 1024 * 1024)
```

## Error Classes

```ruby
ZSV::Error                   # Base error
ZSV::MalformedCSVError       # Parse errors
ZSV::InvalidEncodingError    # Encoding issues
```

## Return Types

- **Without headers**: Rows are `Array<String>`
- **With headers**: Rows are `Hash<String, String>`
- Keys/values are frozen strings for memory efficiency

## Performance Tips

1. **Use foreach for large files** - streaming, O(1) memory
2. **Use parse for small strings** - convenience
3. **Use shift() for control** - pull one row at a time
4. **Reuse parser** - avoid reopening files
5. **Default buffer is optimal** - 256KB balances speed/memory

## Compatibility

Compatible with Ruby's CSV stdlib:

```ruby
# Before
require 'csv'
CSV.foreach("data.csv") { |row| ... }

# After (just change require)
require 'zsv'
ZSV.foreach("data.csv") { |row| ... }
```

Most CSV code works without changes!
