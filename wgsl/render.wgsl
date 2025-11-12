struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f
}

@vertex
fn vertexMain(@builtin(vertex_index) idx: u32) -> VertexOutput {
    var pos = array<vec2f, 6>(
        vec2f(-1.0, -1.0), vec2f(1.0, -1.0), vec2f(-1.0, 1.0),
        vec2f(-1.0, 1.0), vec2f(1.0, -1.0), vec2f(1.0, 1.0)
    );
    var uv = array<vec2f, 6>(
        vec2f(0.0, 1.0), vec2f(1.0, 1.0), vec2f(0.0, 0.0),
        vec2f(0.0, 0.0), vec2f(1.0, 1.0), vec2f(1.0, 0.0)
    );
    var output: VertexOutput;
    output.position = vec4f(pos[idx], 0.0, 1.0);
    output.uv = uv[idx];
    return output;
}

@group(0) @binding(0) var inputTex: texture_2d<f32>;
@group(0) @binding(1) var inputSampler: sampler;

@fragment
fn fragmentMain(input: VertexOutput) -> @location(0) vec4f {
    return textureSample(inputTex, inputSampler, input.uv);
}
