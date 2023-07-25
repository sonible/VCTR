/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "VCTR", "index.html", [
    [ "Getting started", "index.html#autotoc_md11", [
      [ "Dependencies and Requirements", "index.html#autotoc_md12", [
        [ "Apple Accelerate", "index.html#autotoc_md13", null ],
        [ "Intel IPP", "index.html#autotoc_md14", null ]
      ] ],
      [ "Manual Setup", "index.html#autotoc_md15", [
        [ "JUCE", "index.html#autotoc_md16", null ]
      ] ]
    ] ],
    [ "Contributing", "index.html#autotoc_md17", [
      [ "Unit Tests", "index.html#autotoc_md18", [
        [ "Installation via conan", "index.html#autotoc_md19", null ],
        [ "Manual installation", "index.html#autotoc_md20", null ]
      ] ],
      [ "Project structure", "index.html#autotoc_md21", null ],
      [ "Naming conventions", "index.html#autotoc_md22", null ],
      [ "extent", "index.html#autotoc_md23", null ]
    ] ],
    [ "License information", "index.html#autotoc_md24", null ],
    [ "Core Types", "group__Core.html", null ],
    [ "Vector Expressions", "group__Expressions.html", null ],
    [ "blockChannelAsSpan", "namespacevctr_1_1juce__helpers.html#a76d6fc48c137adf182f5310775a73d5e", null ],
    [ "blockChannelAsSpanSIMDAligned", "namespacevctr_1_1juce__helpers.html#a413eb058cc5dffbc2ae6806c65db23ae", null ],
    [ "toSIMDRegister", "namespacevctr_1_1juce__helpers.html#a85dd049e6a77d065f231cf7ccc298c66", null ],
    [ "DataType", "namespacevctr.html#ad3cbb6dc6bd6736802f5a7537b8dfb62", null ],
    [ "ExpressionChainBuilder", "namespacevctr.html#a40c9f56937ddfca3c1b85f5f52627deb", null ],
    [ "FloatType", "namespacevctr.html#aa10a74a826089c483c4eb7feb374f09f", null ],
    [ "RealType", "namespacevctr.html#abbf99aea4bbfba08d526a19614f28516", null ],
    [ "StorageInfoType", "namespacevctr.html#a222cc9e4338c24b2f0751c132503ce3a", null ],
    [ "ValueType", "namespacevctr.html#a83a43fd917a646b18b60be7e88becc33", null ],
    [ "assertCommonSize", "namespacevctr.html#a530eaa0685b6027bcdd79a4ac9a47fda", null ],
    [ "clamp", "group__Expressions.html#gadb6fc764c304eda32439875e7b50d485", null ],
    [ "clampHigh", "group__Expressions.html#gaa42cf0dd0e425bad94a420c9ac6805c7", null ],
    [ "clampLow", "group__Expressions.html#ga69329f63042f587f726465da4ef612d8", null ],
    [ "functionName", "namespacevctr.html#a127327a069f9c9d92bb9c5325234f2fe", null ],
    [ "functionName", "namespacevctr.html#acd3bde90036f67b89e469aea181516e8", null ],
    [ "getCommonExtent", "namespacevctr.html#a50b3b641de5cbcac59ad5c852df70929", null ],
    [ "makeExpressionChainBuilderWithRuntimeArgs", "namespacevctr.html#a732792b914478e386517793a618464ec", null ],
    [ "makeSimdAlignedSpan", "namespacevctr.html#a946767da5d097bc029825bba6c2a2595", null ],
    [ "makeSimdAlignedSpan", "namespacevctr.html#a5e7c9ccef500aff1ca7881f2e1904408", null ],
    [ "makeTemplateExpressionChainBuilderWithRuntimeArgs", "namespacevctr.html#a04278317a44a2211b7c73a529b2692c9", null ],
    [ "map", "group__Expressions.html#gacaebe8e635121b4a8a143abb74bf9fc1", null ],
    [ "map", "group__Expressions.html#ga1bb32794f6c515e3ea8043483bf1dcc9", null ],
    [ "mapFrom0To1", "group__Expressions.html#ga243c68d6c931db606b95fd580f53faec", null ],
    [ "mapFrom0To1", "group__Expressions.html#gab3ac5d8df8839be733d06d9272d7cd25", null ],
    [ "mapTo0To1", "group__Expressions.html#gab4384c06fb1d554d76f83643447159e5", null ],
    [ "mapTo0To1", "group__Expressions.html#ga039f6eeff0c66a878832646d69ee05c8", null ],
    [ "operator*", "group__Expressions.html#ga381264b52d81641d1e0e7fba44b95b33", null ],
    [ "operator*", "group__Expressions.html#gace45c99e6906fb1273db0cfb3ec4fd01", null ],
    [ "operator*", "group__Expressions.html#ga5c66069ba0f2ac2876104893c73a4521", null ],
    [ "operator+", "group__Expressions.html#gad0aa5a68cba8ed5ab033240d238b6002", null ],
    [ "operator+", "group__Expressions.html#ga36f93431c14c872955baaaf0d2052381", null ],
    [ "operator+", "group__Expressions.html#ga24470bf226cfd8eccac84baacc1988be", null ],
    [ "operator-", "group__Expressions.html#ga396d48225010859e9c01a718afd0f922", null ],
    [ "operator-", "group__Expressions.html#ga9174be97e2f8cdaa08c40b5b5f24628b", null ],
    [ "operator-", "group__Expressions.html#gaeaa9cb36a35a3e2be8103e71e6642153", null ],
    [ "operator/", "group__Expressions.html#ga7ca2de641883cfdd1d28b122424864eb", null ],
    [ "operator/", "group__Expressions.html#ga7fa04c18dbf489b3dfccc7d15ea11c2e", null ],
    [ "operator/", "group__Expressions.html#ga33e144c46ba685bf55b8b06fc85e912a", null ],
    [ "operator==", "namespacevctr.html#af90149542071c83fddeb04c75e9cfd3a", null ],
    [ "pow", "group__Expressions.html#ga3b49d9d6f6070cc6a9af10c8c167cc35", null ],
    [ "pow", "group__Expressions.html#ga9bee671c8b09c5aa9d952e8529e47e8b", null ],
    [ "pow", "group__Expressions.html#gacd4fec61c0c7d256340f9483a17d13eb", null ],
    [ "simdAlignedSpanStorageInfo", "namespacevctr.html#abb0c57eb1dc57e153788f2a8631ed6a6", null ],
    [ "sizeToInt", "namespacevctr.html#aecbd148207a93e819482375969ad3db9", null ],
    [ "transformedBy", "group__Expressions.html#gaea7f8d380f1eadb6615c56b8246544d9", null ],
    [ "typeName", "namespacevctr.html#a49c06d6ad88e0ec58063d553c7f15bca", null ],
    [ "abs", "group__Expressions.html#gac4fac892235effd8638c99018b7ab32f", null ],
    [ "acos", "group__Expressions.html#ga778493aa5f46f4a688de257b299c8d92", null ],
    [ "acosh", "group__Expressions.html#ga45af1bb62e14a9d6aa8380346c030c8e", null ],
    [ "angle", "group__Expressions.html#gacdade010cccd89cb91921e3e38c28c55", null ],
    [ "asin", "group__Expressions.html#ga429a6b58b01f054a80d007bc2a6cb7a5", null ],
    [ "asinh", "group__Expressions.html#ga321147aab676531cb4af96f4b4f64d32", null ],
    [ "atan", "group__Expressions.html#ga666c0848887d4feecc228569b1f7a678", null ],
    [ "atanh", "group__Expressions.html#ga9bec0884896ef9a38043781f9a6f7179", null ],
    [ "clampByConstant", "group__Expressions.html#ga9b6f78c2a5e03e76bf2591297c281c4a", null ],
    [ "clampHighByConstant", "group__Expressions.html#ga26405b01dc3bdcb3d46d9f0e5a42340f", null ],
    [ "clampLowByConstant", "group__Expressions.html#gab9cbe3f19b460eac11e8814fb62731cd", null ],
    [ "conjugate", "group__Expressions.html#gaf2cab54fe8d56b64f2cea0d1f2b44529", null ],
    [ "cos", "group__Expressions.html#ga896371d63c7f50b8cf87211c4b07749e", null ],
    [ "cosh", "group__Expressions.html#ga73474b549d4401d5ad5d746dd1b78d49", null ],
    [ "cube", "group__Expressions.html#gad4e424bc5fa6e6b74c38135f20e10621", null ],
    [ "dbToMag", "group__Expressions.html#ga0fb8fcc155a35e271c9698c78fa0fea8", null ],
    [ "dontUseAcceleration", "group__Expressions.html#ga4346d69140e0367d58e03e85c69f7310", null ],
    [ "exp", "group__Expressions.html#ga33d306c075bd8585ee89ab456de95557", null ],
    [ "extentOf", "namespacevctr.html#a0e2879d4e781cb85ff0ae8cf4f1deefd", null ],
    [ "imag", "group__Expressions.html#ga4d940e2110b6323e22d993d3b1e5a63d", null ],
    [ "ln", "group__Expressions.html#ga66fa77725b3684fd045c771c27b7dbac", null ],
    [ "log10", "group__Expressions.html#ga0fd49babf82a9c443081e134f86ee1fc", null ],
    [ "log2", "group__Expressions.html#gace2b11ccc6857b79c0903d3152ea951e", null ],
    [ "magToDb", "group__Expressions.html#ga0dca38713f1fa78037e38766151e7ce6", null ],
    [ "max", "group__Expressions.html#gab29b54656df0ab50dad4271e1addf2c7", null ],
    [ "maxAbs", "group__Expressions.html#gafe2cbadc65ac692fa439cc4c5c18e967", null ],
    [ "mean", "group__Expressions.html#ga5b417f2a488836dd7358dc2b0cadfc10", null ],
    [ "meanSquare", "group__Expressions.html#gaf2a023188f8f05991bf65d56d3127f0d", null ],
    [ "min", "group__Expressions.html#gae77e3e38e1fb46de60f48d2977edc8d5", null ],
    [ "minAbs", "group__Expressions.html#ga3b09e32571fd1b4968f6ba72509a71c7", null ],
    [ "multiplyByConstant", "group__Expressions.html#gacaca8458ca97c6a7da8e682cd6e5d84d", null ],
    [ "normalizeSum", "group__Expressions.html#ga7cccddc76017b29befea7e34bf607010", null ],
    [ "powConstantBase", "group__Expressions.html#ga3b6d257bb352c945c76cae98c730a28b", null ],
    [ "powConstantExponent", "group__Expressions.html#gadc712a3c1ecf10913b0b0be0d2e4f905", null ],
    [ "powerSpectrum", "group__Expressions.html#gaf5ec582bc24648aa5087f73f8eaeb651", null ],
    [ "real", "group__Expressions.html#gae4fc0dae0a600f82c768b555dcf6ea69", null ],
    [ "rms", "group__Expressions.html#ga461b990c1ec58299e9e7bb7ae367fd13", null ],
    [ "sin", "group__Expressions.html#ga179f04cf1507d218f7ed1dd4390af4a5", null ],
    [ "sinh", "group__Expressions.html#gae41920cbcbb09b4745423d71133bd457", null ],
    [ "sqrt", "group__Expressions.html#ga9a6c2e6dc10d5648dd04d6412c376e76", null ],
    [ "square", "group__Expressions.html#gace331be21cfd6aa9705f5bc0e283e3f9", null ],
    [ "sum", "group__Expressions.html#gab068809892f9c3e4f3db4eb1b11abf19", null ],
    [ "tan", "group__Expressions.html#ga8b82fa61931a8f399094c5f9d96305b7", null ],
    [ "tanh", "group__Expressions.html#gafe4e6979458e366c27117a7c163eef06", null ],
    [ "transformedByStaticCastTo", "group__Expressions.html#ga4afc0df5c9d66209b0dec9f4db0515da", null ],
    [ "useAVX", "group__Expressions.html#gaafbd0664a42572849269e06e66088f59", null ],
    [ "useNeon", "group__Expressions.html#gac92083dfec0ef5aeb23135c4fb64fe0c", null ],
    [ "usePlatformVectorOps", "group__Expressions.html#gacf53b60ebe32f078c6fc19ff9c96a2ef", null ],
    [ "useSSE", "group__Expressions.html#ga8e62756910834e1cdc1f0ec4227253aa", null ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ]
      ] ]
    ] ],
    [ "Concepts", "concepts.html", "concepts" ],
    [ "How Expression templates work", "md_include_vctr_Expressions_Readme.html", [
      [ "Creating an <tt>ExpressionChainBuilder</tt> instance", "md_include_vctr_Expressions_Readme.html#autotoc_md1", null ],
      [ "Binary expressions", "md_include_vctr_Expressions_Readme.html#autotoc_md2", null ],
      [ "Using alternative approaches to evaluate the expression", "md_include_vctr_Expressions_Readme.html#autotoc_md3", null ],
      [ "Platform Specific Vector Operations", "md_include_vctr_Expressions_Readme.html#autotoc_md4", null ],
      [ "Reduction expressions", "md_include_vctr_Expressions_Readme.html#autotoc_md5", null ]
    ] ],
    [ "PlatformVectorOps", "md_include_vctr_PlatformVectorOps_Readme.html", [
      [ "Using them in expressions", "md_include_vctr_PlatformVectorOps_Readme.html#autotoc_md7", null ],
      [ "Safe int casting", "md_include_vctr_PlatformVectorOps_Readme.html#autotoc_md8", null ]
    ] ],
    [ "vctr_utils", "md_include_vctr_utils_Readme.html", null ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"classvctr_1_1Range.html#ac938978e7cc55658fb0742613bb9d663",
"conceptvctr_1_1is_1_1suitableForIppComplexFloatBinaryVectorOp.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';