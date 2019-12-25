local sqrt = math.sqrt
local setmetatable = setmetatable
local rawset = rawset
local rawget = rawget


--Vector2====================================================================
vec2 = 
{
--[[	x = 0,
	y = 0,		
	
	class = "vec2",--]]
}

setmetatable(vec2, vec2)

local fields = {}

vec2.__index = function(t,k)
	local var = rawget(vec2, k)
	
	if var == nil then							
		var = rawget(fields, k)
		
		if var ~= nil then
			return var(t)
		end
	end
	
	return var
end

function vec2.new(x, y)
	local v = {x = x or 0, y = y or 0}
	setmetatable(v, vec2)	
	return v
end

function vec2:set(x,y)
	self.x = x or 0
	self.y = y or 0	
end

function vec2:get()
	return self.x, self.y
end

function vec2:sqrMagnitude()
	return self.x * self.x + self.y * self.y
end

function vec2:clone()
	return vec2.new(self.x, self.y)
end

function vec2:normalize()
	local v = self:clone()
	return v:setNormalize()	
end

function vec2:setNormalize()
	local num = self:magnitude()	
	
	if num == 1 then
		return self
    elseif num > 1e-05 then    
        self:div(num)
    else    
        self:set(0,0)
	end 

	return self
end

function vec2.dot(lhs, rhs)
	return lhs.x * rhs.x + lhs.y * rhs.y
end

function vec2.angle(from, to)
	return acos(clamp(vec2.dot(from:normalize(), to:normalize()), -1, 1)) * 57.29578
end


function vec2.magnitude(v2)
	return sqrt(v2.x * v2.x + v2.y * v2.y)
end

function vec2:div(d)
	if type(d) == "number" then
		self.x = self.x / d
		self.y = self.y / d	
	else
		self.x = self.x / d.x
		self.y = self.y / d.y
	end
	
	return self
end

function vec2:mul(d)
	if type(d) == "number" then
		self.x = self.x * d
		self.y = self.y * d	
	else				
		self.x = self.x * d.x
		self.y = self.y * d.y
	end
	
	return self
end

function vec2:add(d)
	self.x = self.x + d.x
	self.y = self.y + d.y

	return self
end

function vec2:sub(d)
	self.x = self.x - d.x
	self.y = self.y - d.y
	
	return
end

function vec2:glm()
	return glmvec2(self.x,self.y)
end

vec2.__tostring = function(self)
	return string.format("[%f,%f]", self.x, self.y)
end

vec2.__div = function(va, d)
	if type(d) == "number" then
		return vec2.new(va.x / d, va.y / d)
	else
		return vec2.new(va.x / d.x, va.y / d.y)
	end
end

vec2.__mul = function(va, d)
	if type(d) == "number" then
		return vec2.new(va.x * d, va.y * d)
	else
		return vec2.new(va.x * d.x, va.y * d.y)
	end
end

vec2.__add = function(va, d)
	return vec2.new(va.x + d.x, va.y + d.y)
end

vec2.__sub = function(va, d)
	return vec2.new(va.x - d.x, va.y - d.y)
end

vec2.__unm = function(va)
	return vec2.new(-va.x, -va.y)
end

vec2.__eq = function(va,d)
	return va.x == d.x and va.y == d.y
end
vec2.fromAngle = function(angle)
	return vec2.new(math.cos(angle),math.sin(angle))
end
function VEC2(x,y)
    return vec2.new(x,y)
end

fields.magnitude 	= vec2.magnitude
fields.normalized 	= vec2.normalize
fields.sqrMagnitude = vec2.sqrMagnitude



--Vector3====================================================================
vec3 = 
{
--[[	x = 0,
	y = 0,		
	
	class = "vec3",--]]
}

setmetatable(vec3, vec3)

local fields = {}

vec3.__index = function(t,k)
	local var = rawget(vec3, k)
	
	if var == nil then							
		var = rawget(fields, k)
		
		if var ~= nil then
			return var(t)
		end
	end
	
	return var
end

function vec3.new(x, y, z)
	local v = {x = x or 0, y = y or 0, z = z or 0}
	setmetatable(v, vec3)	
	return v
end

function vec3:set(x,y,z)
	self.x = x or 0
	self.y = y or 0	
	self.z = z or 0	
end

function vec3:get()
	return self.x, self.y,self.z
end

function vec3:sqrMagnitude()
	return self.x * self.x + self.y * self.y+ self.z * self.z
end

function vec3:clone()
	return vec3.new(self.x, self.y,self.z)
end

function vec3:normalize()
	local v = self:clone()
	return v:setNormalize()	
end

function vec3:setNormalize()
	local num = self:magnitude()	
	
	if num == 1 then
		return self
    elseif num > 1e-05 then    
        self:div(num)
    else    
        self:set(0,0)
	end 

	return self
end

function vec3.dot(lhs, rhs)
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z
end

function vec3.angle(from, to)
	return acos(clamp(vec3.dot(from:normalize(), to:normalize()), -1, 1)) * 57.29578
end


function vec3.magnitude(v2)
	return sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z)
end

function vec3:div(d)
	if type(d) == "number" then
		self.x = self.x / d
		self.y = self.y / d	
		self.z = self.z / d	
	else
		self.x = self.x / d.x
		self.y = self.y / d.y
		self.z = self.z / d.z
	end
	
	return self
end

function vec3:mul(d)
	if type(d) == "number" then
		self.x = self.x * d
		self.y = self.y * d	
		self.z = self.z * d	
	else				
		self.x = self.x * d.x
		self.y = self.y * d.y
		self.z = self.z * d.z
	end
	
	return self
end

function vec3:add(d)
	self.x = self.x + d.x
	self.y = self.y + d.y
	self.z = self.z + d.z

	return self
end

function vec3:sub(d)
	self.x = self.x - d.x
	self.y = self.y - d.y
	self.z = self.z - d.z
	
	return self
end

function vec3:glm()
	return glmvec2(self.x,self.y,self.z)
end

vec3.__tostring = function(self)
	return string.format("[%f,%f,%f]", self.x, self.y,self.z)
end

vec3.__div = function(va, d)
	if type(d) == "number" then
		return vec3.new(va.x / d, va.y / d,va.z / d)
	else
		return vec3.new(va.x / d.x, va.y / d.y,va.z / d.z)
	end
end

vec3.__mul = function(va, d)
	if type(d) == "number" then
		return vec3.new(va.x * d, va.y * d,va.z * d)
	else
		return vec3.new(va.x * d.x, va.y * d.y,va.z * d.z)
	end
end

vec3.__add = function(va, d)
	return vec3.new(va.x + d.x, va.y + d.y, va.z + d.z)
end

vec3.__sub = function(va, d)
	return vec3.new(va.x - d.x, va.y - d.y, va.z - d.z)
end

vec3.__unm = function(va)
	return vec3.new(-va.x, -va.y, -va.z)
end

vec3.__eq = function(va,d)
	return va.x == d.x and va.y == d.y and va.z == d.z
end
function VEC3(x,y,z)
    return vec3.new(x,y,z)
end

fields.magnitude 	= vec3.magnitude
fields.normalized 	= vec3.normalize
fields.sqrMagnitude = vec3.sqrMagnitude

--Vector4====================================================================
vec4 = 
{
--[[	x = 0,
	y = 0,		
	
	class = "vec4",--]]
}

setmetatable(vec4, vec4)

local fields = {}

vec4.__index = function(t,k)
	local var = rawget(vec4, k)
	
	if var == nil then							
		var = rawget(fields, k)
		
		if var ~= nil then
			return var(t)
		end
	end
	
	return var
end

function vec4.new(x, y, z, w)
	local v = {x = x or 0, y = y or 0, z = z or 0,w = w or 0}
	setmetatable(v, vec4)	
	return v
end

function vec4:set(x,y,z,w)
	self.x = x or 0
	self.y = y or 0	
	self.z = z or 0	
	self.w = w or 0	
end

function vec4:get()
	return self.x, self.y,self.z,self.w
end

function vec4:sqrMagnitude()
	return self.x * self.x + self.y * self.y+ self.z * self.z+self.w * self.w
end

function vec4:clone()
	return vec4.new(self.x, self.y,self.z,self.w)
end

function vec4:normalize()
	local v = self:clone()
	return v:setNormalize()	
end

function vec4:setNormalize()
	local num = self:magnitude()	
	
	if num == 1 then
		return self
    elseif num > 1e-05 then    
        self:div(num)
    else    
        self:set(0,0)
	end 

	return self
end

function vec4.dot(lhs, rhs)
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z+ lhs.w * rhs.w
end

function vec4.angle(from, to)
	return acos(clamp(vec4.dot(from:normalize(), to:normalize()), -1, 1)) * 57.29578
end


function vec4.magnitude(v2)
	return sqrt(v2.x * v2.x + v2.y * v2.y + v2.z * v2.z+ v2.w * v2.w)
end

function vec4:div(d)
	if type(d) == "number" then
		self.x = self.x / d
		self.y = self.y / d	
		self.z = self.z / d	
		self.w = self.w / d	
	else
		self.x = self.x / d.x
		self.y = self.y / d.y
		self.z = self.z / d.z
		self.w = self.w / d.w
	end
	
	return self
end

function vec4:mul(d)
	if type(d) == "number" then
		self.x = self.x * d
		self.y = self.y * d	
		self.z = self.z * d	
		self.w = self.w * d	
	else
		self.x = self.x * d.x
		self.y = self.y * d.y
		self.z = self.z * d.z
		self.w = self.w * d.w
	end
	
	return self
end

function vec4:add(d)
	self.x = self.x + d.x
	self.y = self.y + d.y
	self.z = self.z + d.z
	self.w = self.w + d.w

	return self
end

function vec4:sub(d)
	self.x = self.x - d.x
	self.y = self.y - d.y
	self.z = self.z - d.z
	self.w = self.w - d.w

	
	return self
end

function vec4:glm()
	return glmvec2(self.x,self.y,self.z,self.w)
end

vec4.__tostring = function(self)
	return string.format("[%f,%f,%f,%f]", self.x, self.y,self.z,self.w)
end

vec4.__div = function(va, d)
	if type(d) == "number" then
		return vec4.new(va.x / d, va.y / d,va.z / d, va.w / d)
	else
		return vec4.new(va.x / d.x, va.y / d.y,va.z / d.z, va.w / d.w)
	end
end

vec4.__mul = function(va, d)
	if type(d) == "number" then
		return vec4.new(va.x * d, va.y * d,va.z * d, va.w * d)
	else
		return vec4.new(va.x * d.x, va.y * d.y,va.z * d.z, va.w * d.w)
	end
end

vec4.__add = function(va, d)
	return vec4.new(va.x + d.x, va.y + d.y, va.z + d.z, va.w + d.w)
end

vec4.__sub = function(va, d)
	return vec4.new(va.x - d.x, va.y - d.y, va.z - d.zz, va.w - d.w)
end

vec4.__unm = function(va)
	return vec4.new(-va.x, -va.y,-va.z,-va.w)
end

vec4.__eq = function(va,d)
	return va.x == d.x and va.y == d.y and va.z == d.z and va.w == d.w
end
function VEC4(x,y,z,w)
    return vec4.new(x,y,z,w)
end

fields.magnitude 	= vec4.magnitude
fields.normalized 	= vec4.normalize
fields.sqrMagnitude = vec4.sqrMagnitude

