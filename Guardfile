guard :minitest do
    watch(%r{^test/(.*)_spec\.rb$})
end

guard 'rake', :task => 'buildtest' do
    watch %r{ext/pdfium_ext/.*\.(cc|h)$}
end
