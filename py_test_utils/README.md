# Build Test Server
docker build -f ./dockerfile.server -t py_test_server:${version} .

# Build Test Client
docker build -f ./dockerfile.client -t py_test_client:${version} .