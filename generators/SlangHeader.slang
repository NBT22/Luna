module Luna;

[__AttributeUsage(_AttributeTargets.Param)]
public struct Luna_vertexBindingIndexAttribute
{
    uint32_t index;
};

[__AttributeUsage(_AttributeTargets.Param)]
public struct Luna_vertexBindingNameAttribute
{
    string name;
};

[__AttributeUsage(_AttributeTargets.Param)]
struct Luna_VERTEX_INPUT_RATE_VERTEXAttribute {
}

[__AttributeUsage(_AttributeTargets.Param)]
struct Luna_VERTEX_INPUT_RATE_INSTANCEAttribute {
}

