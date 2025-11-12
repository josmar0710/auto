@group(0) @binding(0) var outputTex: texture_storage_2d<rgba32float, write>;

@compute @workgroup_size(8, 8)
fn main(@builtin(global_invocation_id) id: vec3u) {
    let dims = textureDimensions(outputTex);
    if id.x >= dims.x || id.y >= dims.y {
        return;
    }
    let uv = vec2f(f32(id.x) / f32(dims.x), f32(id.y) / f32(dims.y));       
    // Create animated pattern
    let time = f32(id.x + id.y) * 0.01;
    let r = sin(uv.x * 10.0 + time) * 0.5 + 0.5;
    let g = sin(uv.y * 10.0 + time) * 0.5 + 0.5;
    let b = sin((uv.x + uv.y) * 5.0 + time) * 0.5 + 0.5;

    let color = vec4f(r, g, b, 1.0);
    textureStore(outputTex, vec2i(id.xy), color);
}
