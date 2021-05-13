#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"
#include <OCI/Schema1.hpp>
#include <OCI/Schema2.hpp>

// clang-format off
//CMAKE_CPP_EXTRA=${SRC_LIB_DIR}/OCI/Schema1.cpp;${SRC_LIB_DIR}/OCI/Schema2.cpp
// clang-format on

TEST_CASE( "Schema1" ) {
  // The following json is taken: https://docs.docker.com/registry/spec/manifest-v2-1/#example-manifest
  auto im_json = R"(
{
   "name": "hello-world",
   "tag": "latest",
   "architecture": "amd64",
   "fsLayers": [
      {
         "blobSum": "sha256:5f70bf18a086007016e948b04aed3b82103a36bea41755b6cddfaf10ace3c6ef"
      },
      {
         "blobSum": "sha256:5f70bf18a086007016e948b04aed3b82103a36bea41755b6cddfaf10ace3c6ef"
      },
      {
         "blobSum": "sha256:cc8567d70002e957612902a8e985ea129d831ebe04057d88fb644857caa45d11"
      },
      {
         "blobSum": "sha256:5f70bf18a086007016e948b04aed3b82103a36bea41755b6cddfaf10ace3c6ef"
      }
   ],
   "history": [
      {
         "v1Compatibility": "{\"id\":\"e45a5af57b00862e5ef5782a9925979a02ba2b12dff832fd0991335f4a11e5c5\",\"parent\":\"31cbccb51277105ba3ae35ce33c22b69c9e3f1002e76e4c736a2e8ebff9d7b5d\",\"created\":\"2014-12-31T22:57:59.178729048Z\",\"container\":\"27b45f8fb11795b52e9605b686159729b0d9ca92f76d40fb4f05a62e19c46b4f\",\"container_config\":{\"Hostname\":\"8ce6509d66e2\",\"Domainname\":\"\",\"User\":\"\",\"Memory\":0,\"MemorySwap\":0,\"CpuShares\":0,\"Cpuset\":\"\",\"AttachStdin\":false,\"AttachStdout\":false,\"AttachStderr\":false,\"PortSpecs\":null,\"ExposedPorts\":null,\"Tty\":false,\"OpenStdin\":false,\"StdinOnce\":false,\"Env\":[\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\"],\"Cmd\":[\"/bin/sh\",\"-c\",\"#(nop) CMD [/hello]\"],\"Image\":\"31cbccb51277105ba3ae35ce33c22b69c9e3f1002e76e4c736a2e8ebff9d7b5d\",\"Volumes\":null,\"WorkingDir\":\"\",\"Entrypoint\":null,\"NetworkDisabled\":false,\"MacAddress\":\"\",\"OnBuild\":[],\"SecurityOpt\":null,\"Labels\":null},\"docker_version\":\"1.4.1\",\"config\":{\"Hostname\":\"8ce6509d66e2\",\"Domainname\":\"\",\"User\":\"\",\"Memory\":0,\"MemorySwap\":0,\"CpuShares\":0,\"Cpuset\":\"\",\"AttachStdin\":false,\"AttachStdout\":false,\"AttachStderr\":false,\"PortSpecs\":null,\"ExposedPorts\":null,\"Tty\":false,\"OpenStdin\":false,\"StdinOnce\":false,\"Env\":[\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\"],\"Cmd\":[\"/hello\"],\"Image\":\"31cbccb51277105ba3ae35ce33c22b69c9e3f1002e76e4c736a2e8ebff9d7b5d\",\"Volumes\":null,\"WorkingDir\":\"\",\"Entrypoint\":null,\"NetworkDisabled\":false,\"MacAddress\":\"\",\"OnBuild\":[],\"SecurityOpt\":null,\"Labels\":null},\"architecture\":\"amd64\",\"os\":\"linux\",\"Size\":0}\n"
      },
      {
         "v1Compatibility": "{\"id\":\"e45a5af57b00862e5ef5782a9925979a02ba2b12dff832fd0991335f4a11e5c5\",\"parent\":\"31cbccb51277105ba3ae35ce33c22b69c9e3f1002e76e4c736a2e8ebff9d7b5d\",\"created\":\"2014-12-31T22:57:59.178729048Z\",\"container\":\"27b45f8fb11795b52e9605b686159729b0d9ca92f76d40fb4f05a62e19c46b4f\",\"container_config\":{\"Hostname\":\"8ce6509d66e2\",\"Domainname\":\"\",\"User\":\"\",\"Memory\":0,\"MemorySwap\":0,\"CpuShares\":0,\"Cpuset\":\"\",\"AttachStdin\":false,\"AttachStdout\":false,\"AttachStderr\":false,\"PortSpecs\":null,\"ExposedPorts\":null,\"Tty\":false,\"OpenStdin\":false,\"StdinOnce\":false,\"Env\":[\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\"],\"Cmd\":[\"/bin/sh\",\"-c\",\"#(nop) CMD [/hello]\"],\"Image\":\"31cbccb51277105ba3ae35ce33c22b69c9e3f1002e76e4c736a2e8ebff9d7b5d\",\"Volumes\":null,\"WorkingDir\":\"\",\"Entrypoint\":null,\"NetworkDisabled\":false,\"MacAddress\":\"\",\"OnBuild\":[],\"SecurityOpt\":null,\"Labels\":null},\"docker_version\":\"1.4.1\",\"config\":{\"Hostname\":\"8ce6509d66e2\",\"Domainname\":\"\",\"User\":\"\",\"Memory\":0,\"MemorySwap\":0,\"CpuShares\":0,\"Cpuset\":\"\",\"AttachStdin\":false,\"AttachStdout\":false,\"AttachStderr\":false,\"PortSpecs\":null,\"ExposedPorts\":null,\"Tty\":false,\"OpenStdin\":false,\"StdinOnce\":false,\"Env\":[\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\"],\"Cmd\":[\"/hello\"],\"Image\":\"31cbccb51277105ba3ae35ce33c22b69c9e3f1002e76e4c736a2e8ebff9d7b5d\",\"Volumes\":null,\"WorkingDir\":\"\",\"Entrypoint\":null,\"NetworkDisabled\":false,\"MacAddress\":\"\",\"OnBuild\":[],\"SecurityOpt\":null,\"Labels\":null},\"architecture\":\"amd64\",\"os\":\"linux\",\"Size\":0}\n"
      }
   ],
   "schemaVersion": 1,
   "signatures": [
      {
         "header": {
            "jwk": {
               "crv": "P-256",
               "kid": "OD6I:6DRK:JXEJ:KBM4:255X:NSAA:MUSF:E4VM:ZI6W:CUN2:L4Z6:LSF4",
               "kty": "EC",
               "x": "3gAwX48IQ5oaYQAYSxor6rYYc_6yjuLCjtQ9LUakg4A",
               "y": "t72ge6kIA1XOjqjVoEOiPPAURltJFBMGDSQvEGVB010"
            },
            "alg": "ES256"
         },
         "signature": "XREm0L8WNn27Ga_iE_vRnTxVMhhYY0Zst_FfkKopg6gWSoTOZTuW4rK0fg_IqnKkEKlbD83tD46LKEGi5aIVFg",
         "protected": "eyJmb3JtYXRMZW5ndGgiOjY2MjgsImZvcm1hdFRhaWwiOiJDbjAiLCJ0aW1lIjoiMjAxNS0wNC0wOFQxODo1Mjo1OVoifQ"
      }
   ]
})"_json;

  auto im = im_json.get< OCI::Schema1::ImageManifest >();

  REQUIRE( im.name == "hello-world" );
  REQUIRE( im.schemaVersion == 1 );
  REQUIRE( im.architecture == "amd64" );
  REQUIRE( im.tag == "latest" );
  REQUIRE( im.fsLayers[ 0 ].blobSum == "sha256:5f70bf18a086007016e948b04aed3b82103a36bea41755b6cddfaf10ace3c6ef" );
  REQUIRE( im.fsLayers[ 1 ].blobSum == "sha256:5f70bf18a086007016e948b04aed3b82103a36bea41755b6cddfaf10ace3c6ef" );
  REQUIRE( im.fsLayers[ 2 ].blobSum == "sha256:cc8567d70002e957612902a8e985ea129d831ebe04057d88fb644857caa45d11" );
  REQUIRE( im.fsLayers[ 3 ].blobSum == "sha256:5f70bf18a086007016e948b04aed3b82103a36bea41755b6cddfaf10ace3c6ef" );
}


TEST_CASE( "Schema2" ) {
  auto ml_json = R"(
{
  "manifests": [
    {
      "digest": "sha256:d001a8b9d98bfccba9cd5058784d5e5220a85ba950c06abda49837be81ffeebc",
      "mediaType": "application/vnd.docker.distribution.manifest.v2+json",
      "platform": {
        "architecture": "amd64",
        "os": "linux"
      },
      "size": 1160
    }
  ],
  "mediaType": "application/vnd.docker.distribution.manifest.list.v2+json",
  "name": "openshift4/apb-base",
  "originDomain": "registry.redhat.io",
  "raw_str": "{\n    \"manifests\": [\n        {\n            \"digest\": \"sha256:d001a8b9d98bfccba9cd5058784d5e5220a85ba950c06abda49837be81ffeebc\",\n            \"mediaType\": \"application/vnd.docker.distribution.manifest.v2+json\",\n            \"platform\": {\n                \"architecture\": \"amd64\",\n                \"os\": \"linux\"\n            },\n            \"size\": 1160\n        }\n    ],\n    \"mediaType\": \"application/vnd.docker.distribution.manifest.list.v2+json\",\n    \"schemaVersion\": 2\n}",
  "requestedTarget": "4.1",
  "schemaVersion": 2
})"_json;

  auto im_json = R"(
{
  "config": {
    "digest": "sha256:2dd757fce966780e5575d67aef821f91ae4daf40fa6b4c11ce8447ceb6a29c19",
    "mediaType": "application/vnd.docker.container.image.v1+json",
    "size": 5154
  },
  "layers": [
    {
      "digest": "sha256:bb13d92caffa705f32b8a7f9f661e07ddede310c6ccfa78fb53a49539740e29b",
      "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
      "size": 76241735
    },
    {
      "digest": "sha256:455ea8ab06218495bbbcb14b750a0d644897b24f8c5dcf9e8698e27882583412",
      "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
      "size": 1613
    },
    {
      "digest": "sha256:935ce2f796a92f0b6fa7762d7bab7d252ddb59d03cd3afba42d3785b571854f1",
      "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
      "size": 3494835
    },
    {
      "digest": "sha256:8b5e2b6f2e05dc02479437d1414063ba908bbdc22df2830e6d52ba758d7d74e1",
      "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
      "size": 72812309
    }
  ],
  "mediaType": "application/vnd.docker.distribution.manifest.v2+json",
  "name": "openshift4/apb-base",
  "originDomain": "registry.redhat.io",
  "raw_str": "{\n   \"schemaVersion\": 2,\n   \"mediaType\": \"application/vnd.docker.distribution.manifest.v2+json\",\n   \"config\": {\n      \"mediaType\": \"application/vnd.docker.container.image.v1+json\",\n      \"size\": 5154,\n      \"digest\": \"sha256:2dd757fce966780e5575d67aef821f91ae4daf40fa6b4c11ce8447ceb6a29c19\"\n   },\n   \"layers\": [\n      {\n         \"mediaType\": \"application/vnd.docker.image.rootfs.diff.tar.gzip\",\n         \"size\": 76241735,\n         \"digest\": \"sha256:bb13d92caffa705f32b8a7f9f661e07ddede310c6ccfa78fb53a49539740e29b\"\n      },\n      {\n         \"mediaType\": \"application/vnd.docker.image.rootfs.diff.tar.gzip\",\n         \"size\": 1613,\n         \"digest\": \"sha256:455ea8ab06218495bbbcb14b750a0d644897b24f8c5dcf9e8698e27882583412\"\n      },\n      {\n         \"mediaType\": \"application/vnd.docker.image.rootfs.diff.tar.gzip\",\n         \"size\": 3494835,\n         \"digest\": \"sha256:935ce2f796a92f0b6fa7762d7bab7d252ddb59d03cd3afba42d3785b571854f1\"\n      },\n      {\n         \"mediaType\": \"application/vnd.docker.image.rootfs.diff.tar.gzip\",\n         \"size\": 72812309,\n         \"digest\": \"sha256:8b5e2b6f2e05dc02479437d1414063ba908bbdc22df2830e6d52ba758d7d74e1\"\n      }\n   ]\n}",
  "requestedDigest": "sha256:d001a8b9d98bfccba9cd5058784d5e5220a85ba950c06abda49837be81ffeebc",
  "requestedTarget": "4.1",
  "schemaVersion": 2
})"_json;

  auto ml = ml_json.get< OCI::Schema2::ManifestList >();
  auto im = im_json.get< OCI::Schema2::ImageManifest >();

  REQUIRE( ml.mediaType == "application/vnd.docker.distribution.manifest.list.v2+json" );
  REQUIRE( ml.schemaVersion == 2 );
  REQUIRE( ml.originDomain == "" );
  REQUIRE( ml.name == "" );
  REQUIRE( ml.requestedTarget == "" );

  REQUIRE( ml.manifests[ 0 ].digest == "sha256:d001a8b9d98bfccba9cd5058784d5e5220a85ba950c06abda49837be81ffeebc" );
  REQUIRE( ml.manifests[ 0 ].mediaType == "application/vnd.docker.distribution.manifest.v2+json" );
  REQUIRE( ml.manifests[ 0 ].size == 1160 );
  REQUIRE( ml.manifests[ 0 ].platform.architecture == "amd64" );
  REQUIRE( ml.manifests[ 0 ].platform.os == "linux" );
}
