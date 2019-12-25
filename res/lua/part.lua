pos = playerPos()

for i = 1, 100 do
angle = i/50*3.14159
	vel = vec2.fromAngle(angle)*math.random()/3
    world:spawnParticle(Particle.torch_smoke,pos:glm(),vel:glm(),(-vel*0.01):glm(),50+math.random(0,30),-1)
    if math.random(0,1) == 0 then
    world:spawnParticle(Particle.torch_fire,pos:glm(),(vel*0.1):glm(),(-vel*0.001):glm(),50+math.random(0,30),-1)
    end





end