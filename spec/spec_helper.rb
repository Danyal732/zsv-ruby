# frozen_string_literal: true

require 'zsv'
require 'tempfile'

RSpec.configure do |config|
  # Enable flags like --only-failures and --next-failure
  config.example_status_persistence_file_path = '.rspec_status'

  # Disable RSpec exposing methods globally on `Module` and `main`
  config.disable_monkey_patching!

  config.expect_with :rspec do |c|
    c.syntax = :expect
  end
end

# Helper to create temporary CSV files
module CSVHelpers
  def create_csv_file(content)
    file = Tempfile.new(['test', '.csv'])
    file.write(content)
    file.close
    file.path
  end

  def with_csv_file(content)
    path = create_csv_file(content)
    yield path
  ensure
    File.unlink(path) if path && File.exist?(path)
  end
end

RSpec.configure do |config|
  config.include CSVHelpers
end
